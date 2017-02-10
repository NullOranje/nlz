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

    // Compression function
    if (mode == "-z") {
        uint p = 3;  // Max distance pointer
        uint i = 0;  // Last matching index pointer
        uint s = 1;
        char in = (char) input_file.get();
        char n = in;
        dictionary.push_back((const uint &) in);
        dictionary.push_back(i);
        while (in = (char) input_file.get(), input_file.good()) {
            n = in;
            while ((i != s) && (s < p))
                if ((dictionary[s] == in) && (dictionary[s + 1] == i))
                    i = s;
                else
                    s += 2;
            if (s == p) {
                // Now we're at the end of the list, so we can write
                dictionary.push_back((const uint &) n);
                dictionary.push_back(i);
                i = 0;
                p += 2;
                s = 1;
            } else {
                s += 2;
            }
        }

        if (i != 0) {    // This implies that there is nothing to write.
            dictionary.push_back((const uint &) n);
            dictionary.push_back(dictionary[i + 1]);
        }
    }

    /* Write all this stuff to file */
    uint c_bytes = (uint) (ceil((log2(dictionary.size() / 2) - 6) / 8) + 1);

    for (int i = 1; i < dictionary.size(); i += 2) {
        char n = (char) dictionary[i];
        output_file << (char) dictionary[i];
        uint idx = dictionary[i+1];
        uint addl_bytes = 0;
        if (idx > 0) {
            idx = (idx - 1) / 2;
            double bits = log2(idx) + 1;
            if (bits > 6)
                addl_bytes = (uint) ceil((bits - 6) / 8);

        }
        std::stack<char> byte_buffer;
        uint byte_mask = addl_bytes << (6 + addl_bytes * 8);
        idx = idx ^ byte_mask;
        for (int w = 0; w <= addl_bytes; w++) {
            char btw = (char) (idx & 0xff);
            byte_buffer.push(btw);
        }

        while (!byte_buffer.empty()) {
            output_file << byte_buffer.top();
            byte_buffer.pop();
        }
    }

    output_file.close();

    std::cout << "Encoding complete!" << std::endl;
    std::cout << "Dictionary takes " << (uint) log2(dictionary.size() / 2) << " bits." << std::endl;
    std::cout << "Dictionary takes " << c_bytes << " byte(s)." << std::endl;

    for (int i = 0; i < dictionary.size(); i++)
        std::cout << dictionary[i] << " ";

    std::cout << std::endl;

    uint max_l = 0;
    double total = 0.0;
    // Will if float?
    for (uint i = 1; i < dictionary.size(); i += 2) {
        uint s = i;
        uint l = 0;
        std::stack<char> stack;
        while (s != 0) {
            char n = (char) dictionary[s];
            s = dictionary[s + 1];
            stack.push(n);
            l++;
        }

        if (max_l < l) max_l = l;
        total += l;

        while (!stack.empty()) {
            // std::cout << stack.top();
            // output_file << stack.top();
            stack.pop();
        }
        //std::cout << std::endl;
    }

    std::cout << "Dictionary consists of : " << dictionary.size() / 2 << " symbols." << std::endl;
    std::cout << "The average sequence is: " << total / (dictionary.size() / 2) << std::endl;
    std::cout << "The longest sequence is: " <<  max_l << std::endl;

    return 0;
}
