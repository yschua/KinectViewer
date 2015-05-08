#include <iostream>
#include <algorithm>
#include <vector>
#include <queue>
#include <map>
#include <functional>
#include <string>
#include <bitset>
#include <boost\dynamic_bitset.hpp>
typedef unsigned int UINT;
typedef unsigned char BYTE;

struct Node {
  int freq;
  char c;
  Node *left;
  Node *right;
  Node() : c('\0'), left(NULL), right(NULL) {}
  Node(char c, int freq) : c(c), freq(freq), left(NULL), right(NULL) {}
  Node(int freq, Node *left, Node *right) : c('\0'), freq(freq), left(left), right(right) {}
};

// Not used anymore
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

std::string getASCII(char c)
{
  std::bitset<8> ascii(c);
  return ascii.to_string();
}

void getHuffmanCode(Node *node, std::string code, std::map<char, std::string> &huffmanCodes, std::string &huffmanTreeEncoded)
{
  //std::cout << "entering node " << node->c << node->freq << std::endl;
  if (node->left == NULL && node->right == NULL) {
    huffmanTreeEncoded += "1";
    //huffmanTreeEncoded += node->c;
    huffmanTreeEncoded += getASCII(node->c);
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

void reconstructHuffmanTree(SimpleNode *&node, std::string &encodedTree)
{
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

void reconstructHuffmanTree(Node *&node, boost::dynamic_bitset<BYTE> &data)
{
  //UINT lsb = data.size() - 1;
  node = new Node();
  //std::cout << "Data stream: " << data << std::endl;
  //std::cout << "Current bit: " << data[lsb] << std::endl;
  if (data[0] == 0) {
    if (node->left == NULL) {
      data >>= 1;
      data.pop_back();
      reconstructHuffmanTree(node->left, data);
    }
    if (node->right == NULL) {
      data >>= 1;
      data.pop_back();
      reconstructHuffmanTree(node->right, data);
    }
    return;
  } else {
    char ch = 0;
    for (UINT i = 0; i < 8; i++) {
      ch <<= 1;
      data >>= 1;
      data.pop_back();
      ch = ch & 0xfe;
      ch = ch | data[0];
    }
    //std::cout << "Add leaf node: " << ch << std::endl;
    node->c = ch;
    return;
  }
}

int main()
{
  //std::string s = "aaaaabbbbbbbbbccccccccccccdddddddddddddeeeeeeeeeeeeeeeefffffffffffffffffffffffffffffffffffffffffffff";
  //std::string s = "eebbeecdebeeebecceeeddebbbeceedebeeddeeeecceeeedeeedeeebeedeceedebeeedeceeedebee";
  //std::string s = "aaaaaabccccccddeeeee";
  //std::string s = "aabcdef";
  //std::string s = "the quick brown fox jumps over the lazy dog";
  std::string s = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";

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

    minHeap.push(new Node(left->freq + right->freq, left, right));
    //std::cout << "Creating node with f=" << left->freq + right->freq << " left=" << left->c << " right=" << right->c << std::endl;
  }

  // traverse huffman tree to get huffman codes
  // generate huffman tree encoded
  Node *huffmanTree = minHeap.top();
  std::string code("");
  std::string huffmanTreeEncoded("");
  std::map<char, std::string> huffmanCodes;
  getHuffmanCode(huffmanTree, code, huffmanCodes, huffmanTreeEncoded);
  std::reverse(huffmanTreeEncoded.begin(), huffmanTreeEncoded.end());
  //std::cout << "Huffman tree encoded: " << huffmanTreeEncoded << std::endl;

  // calculate huffman code size
  std::string huffmanEncodedMessage("");
  for (UINT i = 0; i < s.length(); i++) {
    huffmanEncodedMessage += huffmanCodes[s[i]];
  }
  std::reverse(huffmanEncodedMessage.begin(), huffmanEncodedMessage.end());
  //std::cout << "Message encoded: " << huffmanEncodedMessage << std::endl;
  std::cout << "Original message size: " << s.length() * 8 << " bits." << std::endl;
  std::cout << "Message size after huffman encoding: " << huffmanEncodedMessage.length() << " bits" << std::endl;
  std::cout << "Header size: " << huffmanTreeEncoded.length() << " bits" << std::endl;
  std::cout << "Total size: " << huffmanEncodedMessage.length() + huffmanTreeEncoded.length() << " bits" << std::endl;

  // convert message and huffman tree string to bit array
  typedef boost::dynamic_bitset<BYTE> Bitset;
  Bitset transmitData(huffmanEncodedMessage + huffmanTreeEncoded);
  std::cout << "Data to transmit: " << transmitData << std::endl;

  //std::vector<unsigned char> bytes;
  //boost::to_block_range(transmitData, std::back_inserter(bytes));
  //unsigned char *byteArray = &bytes[0];
  
  // reconstruct huffman tree from bitset
  Node *reconstructedTree = NULL;
  reconstructHuffmanTree(reconstructedTree, transmitData);
  // dispose last bit
  // TODO: dispose this bit within the function
  transmitData >>= 1;
  transmitData.pop_back();

  // decode remaining data stream
  //std::cout << "Remaining bit stream: " << transmitData << std::endl;
  Node *head = reconstructedTree;
  Node *current = head;
  while (!transmitData.empty()) { // use pseudo EOF
    if (transmitData[0] == 0) {
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
    transmitData >>= 1;
    transmitData.pop_back();
  }
  std::cout << std::endl;


  /*
  // reconstruct huffman tree from string representation
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
  */

  return 0;
}