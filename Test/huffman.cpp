#include <iostream>
#include <algorithm>
#include <vector>
#include <queue>
#include <map>
#include <functional>
#include <string>
typedef unsigned int UINT;

struct Node {
  int freq;
  char c;
  Node *left;
  Node *right;
  Node();
  Node(char c, int freq) : c(c), freq(freq), left(NULL), right(NULL) {}
  Node(char c, int freq, Node *left, Node *right) : c(c), freq(freq), left(left), right(right) {}
};

struct SimpleNode {
  char c;
  SimpleNode *left;
  SimpleNode *right;
  SimpleNode() : c('\0'), left(NULL), right(NULL) {}
};

struct CmpNodePtr {
  bool operator()(const Node *a, const Node *b) const
  {
    return a->freq > b->freq;
  }
};

void getHuffmanCode(Node *node, std::string code, std::map<char, std::string> &huffmanCodes, std::string &huffmanTreeEncoded) {
  //std::cout << "entering node " << node->c << node->freq << std::endl;
  if (node->left == NULL && node->right == NULL) {
    huffmanTreeEncoded += "1";
    huffmanTreeEncoded += node->c;
    std::cout << node->c << ": " << code << std::endl;
    huffmanCodes[node->c] = code;
  } else {
    huffmanTreeEncoded += "0";
    if (node->left != NULL) {
      //std::cout << "entering left" << std::endl;
      getHuffmanCode(node->left, code + "0", huffmanCodes, huffmanTreeEncoded);
    }
    if (node->right != NULL) {
      //std::cout << "entering right" << std::endl;
      getHuffmanCode(node->right, code + "1", huffmanCodes, huffmanTreeEncoded);
    }
  }
  return;
}

void reconstructHuffmanTree(SimpleNode *&node, std::string &encodedTree) {
  //std::cout << "Reading: " << encodedTree[0] << std::endl;
  node = new SimpleNode();
  if (encodedTree[0] == '0') {
    //std::cout << "Create internal node" << std::endl;  
    if (node->left == NULL) {
      //std::cout << "Go to left child" << std::endl;
      //node->left = new SimpleNode();
      encodedTree.erase(0, 1);
      reconstructHuffmanTree(node->left, encodedTree);
    }
    if (node->right == NULL) {
      //std::cout << "Go to right child" << std::endl;
      //node->right = new SimpleNode();
      encodedTree.erase(0, 1);
      reconstructHuffmanTree(node->right, encodedTree);
    }
    return;
  } else {
    //std::cout << "Create leaf node: " << encodedTree[1] << std::endl;
    node->c = encodedTree[1];
    encodedTree.erase(0, 1);
    return;
  }
}

int main()
{
  //std::string s = "aaaaabbbbbbbbbccccccccccccdddddddddddddeeeeeeeeeeeeeeeefffffffffffffffffffffffffffffffffffffffffffff";
  //std::string s = "eebbeecdebeeebecceeeddebbbeceedebeeddeeeecceeeedeeedeeebeedeceedebeeedeceeedebee";
  std::string s = "aabcdef";
  std::cout << "Original string size: " << s.length() * 8 << " bits." << std::endl;

  // find frequency of characters
  std::map<char, int> charFreq;
  for (UINT i = 0; i < s.length(); i++) {
    charFreq[s[i]]++;
  }

  // store in min heap
  std::priority_queue<Node *, std::vector<Node *>, CmpNodePtr> minHeap;
  for (std::map<char, int>::iterator it = charFreq.begin(); it != charFreq.end(); it++) {
    Node *node = new Node(it->first, it->second);
    minHeap.push(node);
  }

  //while (!minHeap.empty()) {
  //  Node *topNode = minHeap.top();
  //  std::cout << topNode->c << ": " << topNode->freq << std::endl;
  //  minHeap.pop();
  //}

  // generate huffman tree
  while (minHeap.size() != 1) {
    Node *right = minHeap.top();
    minHeap.pop();
    Node *left = minHeap.top();
    minHeap.pop();

    minHeap.push(new Node('\0', left->freq + right->freq, left, right));
    //std::cout << "Creating node with f=" << left->freq + right->freq << " left=" << left->c << " right=" << right->c << std::endl;
  }

  // traverse huffman tree to get huffman codes
  // generate huffman tree encoded
  Node *huffmanTree = minHeap.top();
  std::string code("");
  std::string huffmanTreeEncoded("");
  std::map<char, std::string> huffmanCodes;
  getHuffmanCode(huffmanTree, code, huffmanCodes, huffmanTreeEncoded);
  std::cout << "Huffman tree encoded: " << huffmanTreeEncoded << std::endl;

  // calculate huffman code size
  std::string huffmanEncodedMessage("");
  for (UINT i = 0; i < s.length(); i++) {
    huffmanEncodedMessage += huffmanCodes[s[i]];
  }
  UINT messageSize, headerSize;
  std::cout << "Message size after huffman encoding: " << (messageSize = huffmanEncodedMessage.length()) << " bits" << std::endl;
  std::cout << "Header size: " << (headerSize = huffmanTreeEncoded.length() - huffmanCodes.size() + (huffmanCodes.size() * 8)) << " bits" << std::endl;
  std::cout << "Total size: " << messageSize + headerSize << " bits" << std::endl;

  // reconstruct huffman tree
  SimpleNode *reconstructedTree = NULL;
  reconstructHuffmanTree(reconstructedTree, huffmanTreeEncoded);

  // decode a huffman code with reconstructed tree
  code = "000000000000";
  SimpleNode *head = reconstructedTree;
  SimpleNode *current = head;
  for (UINT i = 0; i < code.length(); i++) {
    if (code[i] == '0') {
      if (current->left->c != '\0') {
        std::cout << current->left->c;
        current = head;
      } else {
        current = current->left;
      }
    } else {
      if (current->right->c != '\0') {
        std::cout << current->right->c;
        current = head;
      } else {
        current = current->right;
      }
    }
  }
  std::cout << std::endl;

  return 0;
}