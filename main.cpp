#include <iostream>
#include <boost/container/stable_vector.hpp>
#include <fstream>
#include <stack>
#include <vector>
#include <cmath>

void print_usage() {
    std::cout << "Usage: nlz [-z compress | -x decompress] input_file output_file" << std::endl;
}

int main(int argc, char *argv[]) {
    std::vector<uint> dictionary(1);
    //boost::container::stable_vector<uint> dictionary(1);
    std::ifstream input_file;
    std::ofstream output_file;
    std::string mode;

    if (argc == 4) {
        mode.assign(argv[1]);
        if (!((mode == "-z") || (mode == "-x"))) {
            std::cout << "Illegal argument." << std::endl;
            print_usage();
            exit(1);
        }
        input_file.open(argv[2]);
        if (input_file.fail()) {
            std::cout << "File I/O failure.  Quitting." << std::endl;
            exit(1);
        }
        output_file.open(argv[3]);
    } else {
        print_usage();
    }

    if (mode == "-z") {     // Compression function
        uint p = 2;  // Max distance pointer
        uint i = 0;  // Last matching index pointer
        uint s = 2;
        uint in;
        uint n;
        while (in = (uint) input_file.get(), input_file.good()) {
            n = in;
            while ((i != s) && (s < p))
                if ((dictionary[s - 1] == in) && (dictionary[s] == i))
                    i = s;
                else
                    s += 2;
            if (s == p) {
                // Now we're at the end of the list, so we can write
                dictionary.push_back(n);
                dictionary.push_back(i);
                i = 0;
                p += 2;
                s = 2;
            } else {
                s += 2;
            }
        }

        if (i != 0) {    // This implies that there is nothing to write.
            dictionary.push_back((const uint &) n);
            dictionary.push_back(dictionary[i + 1]);
        }

        // Output the dictionary for reference.  Debugging only
        for (int i = 1; i < dictionary.size(); i++) {
            std::cout << dictionary[i] << " ";
            if (i % 2 == 0)
                std::cout << std::endl;
        }

        output_file << (char) 0xff;
        bool is_char = true;
        for (int c = 1; c < dictionary.size(); c++) {
            if (is_char) {
                output_file << (char) dictionary[c];
            } else {
                auto idx = dictionary[c] / 2;
                double bits = log2(idx);
                uint addl_bytes = 0;
                if (bits > 6)
                    addl_bytes = (uint) ceil((bits - 6) / 8);
                uint byte_mask = addl_bytes << (6 + addl_bytes * 8);
                idx = idx ^ byte_mask;
                if (idx == 0) {
                    output_file << (char) idx;
                } else {
                    std::stack<char> byte_buffer;
                    while (idx > 0) {
                        char btw = (char) (idx & 0xff);
                        byte_buffer.push(btw);
                        idx = idx >> 8;
                    }
                    while (!byte_buffer.empty()) {
                        output_file << byte_buffer.top();
                        byte_buffer.pop();
                    }
                }
            }
            is_char = !is_char;
        }

        output_file.close();

        uint c_bytes = (uint) (ceil((log2(dictionary.size() / 2) - 6) / 8) + 1);
        std::cout << "Encoding complete!" << std::endl;
        std::cout << "Dictionary takes " << (uint) log2(dictionary.size() / 2) << " bits." << std::endl;
        std::cout << "Dictionary takes " << c_bytes << " byte(s)." << std::endl;
        std::cout << "Dictionary consists of : " << dictionary.size() / 2 << " symbols." << std::endl;

    } else if (mode == "-x") {      // Decompression function
        // First thing's first: build our dictionary
        int c = input_file.get();
        if (c != 0xff) {
            std::cerr << "Invalid file.  Quitting." << std::endl;
            // return 13;
        }
        dictionary[0] = 0;
        bool is_symbol = true;
        uint in;
        while (in = (uint) input_file.get(), input_file.good()) {
            if (is_symbol) {
                dictionary.push_back(in);
                is_symbol = false;
            } else {
                uint addl_bytes = (uint) ((in & 0xC0) >> 6);
                in &= 0x3F;
                for (int i = 0; i < addl_bytes; i++) {
                    in = in << 8;
                    uint t = (uint) input_file.get();
                    in += t;
                }
                uint idx = (uint) in * 2;
                dictionary.push_back(idx);
                is_symbol = true;
            }
        }

        // Will if float?
        for (uint i = 2; i < dictionary.size(); i += 2) {
            uint s = i;
            uint l = 0;
            std::stack<char> stack;
            while (s > 0) {
                char n = (char) dictionary[s - 1];
                std::cout << s <<  " ";
                s = dictionary[s];
                stack.push(n);
                l++;
            }
            std::cout << std::endl;
            while (!stack.empty()) {
                // std::cout << stack.top();
                output_file << stack.top();
                stack.pop();
            }

        }
        std::cout << std::endl;
    }

    return 0;
}