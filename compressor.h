#ifndef COMPRESSOR_H
#define COMPRESSOR_H
#include <iostream>
#include <fstream>
#include <map>
#include <queue>
#include <vector>
#include <bitset>
#include <QFileInfo>
#include <QDebug>
#include "md5.h"

struct haffNode {
    unsigned long long freq;
    unsigned char uchar;
    string code;
    struct haffNode* left = 0;
    struct haffNode* right = 0;
};
struct cmp {
    bool operator ()(const haffNode* a, const haffNode* b) {
        return a->freq > b->freq;
    }
};
class Compressor
{
public:
    Compressor();
    map<unsigned char, string> codeMap;
    //压缩
    int compress(string sourcePath, string destinationPath, string pw = "");
    //编码
    void encode(haffNode* pn, string code);
    //插入node
    void insert_node(haffNode* father, unsigned char uchar, string code) {
        if (code.empty()) {
            father->uchar = uchar;
            return;
        }
        char way = code[0];
        if (way == '0') {
            if (!(father->left)) {
                haffNode* son = new (haffNode);
                father->left = son;
            }
            insert_node(father->left, uchar, code.substr(1));
        } else {
            if (!(father->right)) {
                haffNode* son = new (haffNode);
                father->right = son;
            }
            insert_node(father->right, uchar, code.substr(1));
        }
    }
    //解压
    int decompress(string sourcePath, string destinationPath, string pw = "");
};

#endif // COMPRESSOR_H
