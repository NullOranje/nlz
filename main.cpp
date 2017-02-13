#include <iostream>
#include <fstream>
#include <stack>
#include <vector>
#include <unordered_map>
#include <map>
#include <chrono>

void print_usage() {
    std::cout << "Usage: nlz [-z compress | -x decompress] input_file output_file" << std::endl;
}

int main(int argc, char *argv[]) {
    std::vector<uint> dictionary(1);
    std::map<std::string, uint> cache;
    std::ifstream input_file;
    std::ofstream output_file;
    std::string mode;

    /* Command line argument parsing */
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

    auto start = std::chrono::high_resolution_clock::now();
    if (mode == "-z") {     // Compression function
        uint p = 2;  // Max distance pointer
        uint i = 0;  // Last matching index pointer
        uint s = 2;
        unsigned char in;
        unsigned char n;
        std::string cache_string;
        while (in = (unsigned char) input_file.get(), input_file.good()) {
            n = in;
            cache_string.push_back(in);

            // Check the cache to see if we've seen this string before
            auto cache_lookup = cache.find(cache_string);
            if (cache_lookup == cache.end()) {
                while ((i != s) && (s < p))
                    if ((dictionary[s - 1] == in) && (dictionary[s] == i))
                        i = s;
                    else
                        s += 2;
            } else {
                i = cache_lookup->second;
                s = i;
            }

            if (s == p) {
                // Now we're at the end of the list, so we can write
                dictionary.push_back(n);
                dictionary.push_back(i);
                cache[cache_string] = s;
                i = 0;
                p += 2;
                s = 2;
                cache_string.clear();

            } else {
                s += 2;
            }
        }

        if (i != 0) {    // This implies that there is nothing to write.
            dictionary.push_back(n);
            dictionary.push_back(dictionary[i]);
        }

        output_file << (unsigned char) 0xff;
        bool is_char = true;
        for (int c = 1; c < dictionary.size(); c++) {
            if (is_char) {
                output_file << (unsigned char) dictionary[c];
            } else {
                auto idx = dictionary[c] / 2;
                double bits = log2(idx) + 1;
                uint addl_bytes = 0;
                if (bits > 6)
                    addl_bytes = (uint) ceil(((bits - 6)) / 8);
                uint byte_mask = addl_bytes << (6 + addl_bytes * 8);
                idx = idx ^ byte_mask;

                if (idx == 0) {
                    output_file << (unsigned char) idx;
                } else {
                    std::stack<unsigned char> byte_buffer;
                    while (idx > 0) {
                        unsigned char btw = (unsigned char) (idx & 0xff);
                        byte_buffer.push(btw);
                        idx = idx >> 8;
                    }
                    while (!byte_buffer.empty()) {
                        output_file << (unsigned char) byte_buffer.top();
                        byte_buffer.pop();
                    }
                }
            }
            is_char = !is_char;
        }

        output_file.close();

        // Some statistics
        uint c_bytes = (uint) (ceil((log2(dictionary.size() / 2) - 6) / 8) + 1);
        std::cout << "Encoding complete!" << std::endl;
        std::cout << "Dictionary takes " << (uint) log2(dictionary.size() / 2) << " bits." << std::endl;
        std::cout << "Dictionary takes " << c_bytes << " byte(s)." << std::endl;
        std::cout << "Dictionary consists of : " << dictionary.size() / 2 << " symbols." << std::endl;
        std::cout << "Dictionary.size(): " << dictionary.size() << std::endl;

    } else if (mode == "-x") {      // Decompression function
        // First thing's first: build our dictionary
        int c = input_file.get();
        if (c != 0xff) {
            std::cerr << "Invalid file.  Quitting." << std::endl;
            // return 13;
        }
        dictionary[0] = 0;
        bool is_symbol = true;
        unsigned int in;
        int counter = 1;
        int offset = 0;
        while (in = (unsigned char) input_file.get(), input_file.good()) {
            offset++;
            if (is_symbol) {
                dictionary.push_back(in);
                counter++;
            } else {
                uint addl_bytes = (uint) ((in & 0xC0) >> 6);
                in &= 0x3F;
                uint t = in;
                for (int i = 0; i < addl_bytes; i++) {
                    t = t << 8;
                    in = (unsigned char) input_file.get();
                    t += in;
                    offset++;
                }
                uint idx = (uint) t * 2;
                dictionary.push_back(idx);
            }
            is_symbol = !is_symbol;
            offset += 1;
        }

        // Will if float?
        for (uint i = 2; i < dictionary.size(); i += 2) {
            uint s = i;
            uint l = 0;
            std::stack<char> stack;
            while (s > 0) {
                char n = (char) dictionary[s - 1];
                s = dictionary[s];
                stack.push(n);
                l++;
            }
            while (!stack.empty()) {
                output_file << stack.top();
                stack.pop();
            }
        }
    }

    auto stop = std::chrono::high_resolution_clock::now();
    std::cout << "Running time: " << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() << "ms." << std::endl;

    return 0;
}