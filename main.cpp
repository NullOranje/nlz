#include <iostream>
#include <boost/container/stable_vector.hpp>
#include <fstream>
#include <stack>
#include <vector>

void print_usage() {
    std::cout << "Usage: nlz [-z compress | -x decompress] input_file output_file" << std::endl;
}

int main(int argc, char *argv[]) {
    // std::vector<uint> dictionary(0);
    boost::container::stable_vector<uint> dictionary(0);
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
        uint p = 2;  // Max distance pointer
        uint i = UINT_MAX;  // Last matching index pointer
        uint s = 0;
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
                i = UINT_MAX;
                p += 2;
                s = 0;
            } else {
                s += 2;
            }
        }

        if (i != UINT_MAX) {    // This implies that there is nothing to write.
            dictionary.push_back((const uint &) n);
            dictionary.push_back(dictionary[i + 1]);
        }
    }

    std::cout << "Encoding complete!" << std::endl;

    uint max_l = 0;
    double total = 0.0;
    // Will if float?
    for (uint i = 0; i < dictionary.size(); i += 2) {
        uint s = i;
        uint l = 0;
        std::stack<char> stack;
        while (s != UINT_MAX) {
            char n = (char) dictionary[s];
            s = dictionary[s + 1];
            stack.push(n);
            l++;
        }

        if (max_l < l) max_l = l;
        total += l;

        while (!stack.empty()) {
            std::cout << stack.top();
            output_file << stack.top();
            stack.pop();
        }
        std::cout << std::endl;
    }

    std::cout << "Dictionary consists of : " << dictionary.size() / 2 << " symbols." << std::endl;
    std::cout << "The average sequence is: " << total / (dictionary.size() / 2) << std::endl;
    std::cout << "The longest sequence is: " <<  max_l << std::endl;

    return 0;
}
