//
//  main.cpp
//  HuffmanCoding
//
//  Created by chenqiu on 11/24/17.
//  Copyright © 2017 chenqiu. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <math.h>
using namespace std;

#define DECODE_EOF  0x100
#define chara_type   256 + 1
#define FILE_PATH   "/Users/chenqiu/Downloads/Huffman_File/Golden_Gate_Bridge,_SF_(cropped).bmp"
#define ENCODE_SUFFIX   ".zip.huff"

class MinHeapNode{
public:
    MinHeapNode *left;
    MinHeapNode *right;
    unsigned weight;
    unsigned short ch;
    MinHeapNode(unsigned weight, MinHeapNode *lnode, MinHeapNode *rnode, short c){
        this->weight = weight;
        this->left = lnode;
        this->right = rnode;
        this->ch = c;
    }
};

class MinHeap{
private:
    void insertNode(int ch, int weight){
        int low = 0, high = size - 1, mid = ceil((low + high)/2);
        while (low <= high) {
            if (array[mid]->weight > weight) high = mid - 1;
            else low = mid + 1;
            mid = ceil((low + high) / 2);
        }
        for (int i = size - 1; i >= low; i--)
            array[i + 1] = array[i];
        array[low] = new MinHeapNode(weight, NULL, NULL, ch);
        size++;
    }
    void insertNode(MinHeapNode *node){
        int low = 0, high = size - 1, mid = ceil((low + high)/2);
        //binary search
        while (low <= high) {
            if (array[mid]->weight > node->weight) high = mid - 1;
            else low = mid + 1;
            mid = ceil((low + high) / 2);
        }
        for (int i = capacity - 2; i >= low; i--)
            array[i + 1] = array[i];
        array[low] = node;
        size++;
    }
    MinHeapNode* extractMinNode(){
        MinHeapNode *temp = array[0];
        for (int i = 0; i < size - 1; i++)
            array[i] = array[i + 1];
        array[size - 1] = temp;
        size --;
        return temp;
    }
public:
    MinHeapNode **array;
    unsigned size; // current size of heap
    unsigned capacity;
    MinHeap(unsigned capacity, const int arr[]){
        this->capacity = capacity;
        array = (MinHeapNode **)malloc(sizeof(MinHeapNode *) * (capacity));
        size = 0;
        int len = chara_type;
        for (int i = 0; i < len; i++)
            if (arr[i] > 0)
                insertNode(i, arr[i]);
        while (size > 1) {
            MinHeapNode *child_l = extractMinNode();
            MinHeapNode *child_r = extractMinNode();
            MinHeapNode *p = new MinHeapNode(child_l->weight + child_r->weight, child_l, child_r, 0xffff);
            insertNode(p);
        }
    }
    MinHeap(unsigned leaf_num, const short leaf_ch[], const string encode[]){ //positive create
        array = new MinHeapNode*[2 * leaf_num - 1];
        array[0] = new MinHeapNode(0, NULL, NULL, NULL); //root
        int idx = 0;
        for (int i = 0; i < leaf_num; i++){
            MinHeapNode *p = array[0];
            for (int j = 0; j < encode[i].length(); j++){
                if (encode[i][j] == '0'){
                    if (p->left == NULL){
                        array[++ idx] = new MinHeapNode(0, NULL, NULL, NULL);
                        p->left = array[idx];
                    }
                    p = p->left;
                }
                else {
                    if (p->right == NULL){
                        array[++ idx] = new MinHeapNode(0, NULL, NULL, NULL);
                        p->right = array[idx];
                    }
                    p = p->right;
                }
            }
            p->ch = leaf_ch[i];
        }
    }
    MinHeapNode *root(){
        return array[0];
    }
};

class Huffman{
private:
    string file_name;
    int count_char[chara_type];
    int leaf_num;
    string *encode_str;
    void createHuffmanTree();
    void rebuildHuffmanTree(const short data[], const string encode[]);
public:
    MinHeap *huffman_tree;
    Huffman(int leaf_num);
    Huffman(string file_name);
    static void parse_file(string file_name, int count_char[]);
    void huffman_encode(string in_file_name, string out_file_name);
    static void huffman_decode(string decode_input_file, string decode_out_file);
    void print_tree(MinHeapNode *next, string temp_str);
};

void Huffman::createHuffmanTree(){
    leaf_num = 0;
    for (int i = 0; i < chara_type; i++)
        if (count_char[i] > 0) { leaf_num++;         //   cout << i << ":" << count_char[i] << endl;
        }
    encode_str = new string[chara_type];
    clock_t start, end;
    start = clock();
    
    huffman_tree = new MinHeap(2 * leaf_num - 1, count_char);  //build tree
    print_tree(huffman_tree->root(), ""); // encode the tree
    
    end = clock();
    cout << "build tree cost: " <<(double)(end - start) / CLOCKS_PER_SEC << "s" << endl;
}

Huffman::Huffman(string file_name){
    this->file_name = file_name;
    clock_t start = clock(), end;
    Huffman::parse_file(file_name, count_char);
    end = clock();
    cout << "character counted cost: " << (double)(end - start) / CLOCKS_PER_SEC << "s" << endl;
    count_char[DECODE_EOF] = 1; //file eof freq = 1
}

Huffman::Huffman(int leaf_num){
    this->leaf_num = leaf_num;
    encode_str = new string[chara_type];
    memset(count_char, 0, sizeof(count_char));
}

void Huffman::huffman_encode(string in_file_name, string out_file_name){
    createHuffmanTree();
    clock_t start = clock(), end;
    ofstream out(out_file_name, ios::out);
    if (!out.is_open())
        exit(-1);
    out << leaf_num << endl; // write in short type (> 256)
    for (int i = 0; i < chara_type; i++)
        if (count_char[i] > 0)
            out << i << " " << encode_str[i] << endl;
    
    char buff_bits = 0;
    string buff_str = "", code = "";
    int str_len = 10;
    ifstream in(in_file_name, ios::in);
    if (!in.is_open()){
        unsigned long index = file_name.find_last_of("\\");
        if (index == -1)
            index = file_name.find_last_of("/");
        cout << file_name.substr(index + 1, file_name.length() - 1) << " cannot open." << endl;
        exit(-1);
    }
    while (!in.eof()) {
        int ch = in.get();
        code += encode_str[ch];
        if (in.eof())
            code += encode_str[DECODE_EOF];
        if (code.length() >= 8){
            int i;
            for (i = 0; i + 7 < code.length(); i += 8){
                for (int j = 0; j < 8; j++){
                    if (code[i + j] == '1')
                        buff_bits |= 1 << (7 - j); //inverse
                }
                buff_str += buff_bits;
                buff_bits = 0;
            }
            code = code.substr(i, code.length() - 1);
            if (buff_str.length() > str_len){
                out << buff_str;
                buff_str = "";
            }
        }
    }
    //the last filled with 0
    for (int i = 0; i < code.length(); i++)
        if (code[i] == '1')
            buff_bits |= 1 << (7 - i);
    out << buff_str + buff_bits;
    in.close();
    out.flush();
    out.close();
    end = clock();
    cout << "compress end,total cost: " << (double)(end - start) / CLOCKS_PER_SEC << "s" << endl;
}

void Huffman::huffman_decode(string decode_input_file, string decode_out_file){
    ifstream in(decode_input_file, ios::in);
    ofstream out(decode_out_file, ios::out);
    if (!in.is_open()){
        cout << decode_input_file.substr(decode_input_file.find_last_of("/") + 1, decode_input_file.length() - 1) << " cannot open." << endl;
        exit(-1);
    }
    if (!out.is_open()){
        cout << decode_out_file.substr(decode_out_file.find_last_of("/") + 1, decode_out_file.length() - 1) << " cannot open." << endl;
        exit(-1);
    }
//    int leaf = in.get();
    int leaf;
    in >> leaf;
    in.get(); // endl
    short *leaf_data = new short[leaf];
    string *encode = new string[leaf];
    int t = 0;
    while (t < leaf) {
        in >> leaf_data[t];
        in >> encode[t];
        t++;
    }
    Huffman *huff = new Huffman(leaf);

    clock_t start = clock(), end;
    huff->rebuildHuffmanTree(leaf_data, encode);
    end = clock();
    cout << "rebuild huffman tree cost: " << (double)(end - start) / CLOCKS_PER_SEC << "s" << endl;
    
    in.get(); //remove the last \n
    int ch = in.get();
    int buff_size = 10;
    int idx = 0;
    string buff_str = "";
    while (true) {
        MinHeapNode *p = huff->huffman_tree->root();
        short decode = -1;
        while (p) {
            for (; idx < 8 && p; idx++){
                if (ch & (0x80 >> idx)){ // 1
                    if (p->right == NULL){
                        decode = p->ch;
                        idx --;
                    }
                    p = p->right;
                }
                else { // 0
                    if (p->left == NULL){
                        decode = p->ch;
                        idx --;
                    }
                    p = p->left;
                }
            }
            if(idx >= 8) {
                idx = 0;
                ch = in.get();
            }
        }
        if (decode == DECODE_EOF){
            out << buff_str;
            buff_str = "";
            break;
        }
        buff_str += (char)decode;
        if (buff_str.length() > buff_size){
            out << buff_str;
            buff_str = "";
        }
    }
    out.flush();
    out.close();
    
    in.close();
    cout << "extract end,total cost: " << (double)(clock() - start) / CLOCKS_PER_SEC << "s" << endl;
}

void Huffman::rebuildHuffmanTree(const short data[], const string encode[]){
    huffman_tree = new MinHeap(leaf_num, data, encode);
}

void Huffman::print_tree(MinHeapNode *next, string temp_str){
    if (next == NULL)
        return;
    if (next->ch != 0xffff){
        encode_str[next->ch] = temp_str;
//        cout << temp_str;
//        cout << " " << (char)next->ch << endl;
        return;
    }
    temp_str += "0";
    print_tree(next->left, temp_str);
    temp_str.pop_back();
    temp_str += "1";
    print_tree(next->right, temp_str);
    temp_str.pop_back();
}

void Huffman::parse_file(string file_name, int count_char[]){
    ifstream in(file_name);
    if (!in.is_open()){
        unsigned long index = file_name.find_last_of("\\");
        if (index == -1)
            index = file_name.find_last_of("/");
        cout << file_name.substr(index + 1, file_name.length() - 1) << " cannot open." << endl;
        exit(-1);
    }
    memset(count_char, 0, sizeof((char *)count_char));
    while(!in.eof()){
        int ch = in.get();
        count_char[ch]++;
    }
    in.close();
}

class A{
public:
    int n;
    A(int n){
        this->n = n;
    }
};

void testA(){
    //    A **arr = new A*[5];
    //    for (int i = 0; i < 3; i++)
    //        arr[i] = new A(i);
    //    for (int i = 3; i >= 0; i--) {
    //        arr[i + 1] = arr[i];
    //    }
    //    arr[0] = new A(-1);
    //    for (int i = 0; i < 4; i++)
    //        cout << arr[i]->n << endl;
}

int main(int argc, const char * argv[]) {
    string name = FILE_PATH;
    name = name.substr(name.find_last_of("/") + 1, name.length() - 1);
    string enc_file = "/users/chenqiu/downloads/Huffman_File/enc_" + name;
    string dec_file = "/users/chenqiu/downloads/Huffman_File/dec_" + name;
    
    Huffman* huff = new Huffman(FILE_PATH);
    huff->huffman_encode(FILE_PATH, enc_file + ENCODE_SUFFIX);
    huff->huffman_decode(enc_file + ENCODE_SUFFIX, dec_file);

    return 0;
    if (argc == 1){
        cout << "please input file name(.*)" << endl;
        return -1;
    }
    else{
        for (int i = 1; i < argc; i++){
            string source_path = *new string(argv[i]);
            string name = "";
            unsigned long index;
            if ((index = source_path.find_last_of("\\")) != -1);
            else if ((index = source_path.find_last_of("/")) != -1);
            else
                index = -1;
            name = source_path.substr(index + 1, source_path.length() - 1);
            string err_file = "";
            err_file += name[name.length() - 4];
            err_file +=  name[name.length() - 3];
            err_file += name[name.length() - 2];
            err_file += name[name.length() - 1];
            if (err_file == ".rar" || err_file == ".zip"){
                cout << "i cannot extract " << err_file << endl;
                return -1;
            }
            Huffman *huff = new Huffman(source_path);
            if (err_file == "huff"){
                string type = ".zip.huff";
                huff->huffman_decode(source_path, source_path.substr(0, index) + "dec_" + name.substr(0, name.find_last_of(type) - type.length() + 1));
                cout << "ok, " << name << " extracted." << endl;
            }
            else{
                string type = ".zip.huff";
                huff->huffman_encode(source_path, name + type);
                cout << "ok, " << name << " compressed." << endl;
            }
        }
    }
    return 0;
}
