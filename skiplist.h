#include <iostream>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <mutex>
#include <fstream>

#define STORE_FILE "store/dumpFile"

std::mutex mtx, mtx1;
std::string delimiter = ":";

// 跳表节点结构体定义
template<typename K, typename V>
class Node {
public:
    Node() {}                   // 默认构造函数
    Node(K k, V v, int);        // 构造函数
    ~Node();                    // 析构函数

    K get_key() const;          // 取 Key
    V get_value() const;        // 取 value
    void set_value(V);          // 设定 value

    // 二重指针，一块内存forwad 里面存储了Node* 的指针
    Node<K, V>** forward;
    int node_level;             // 层数

private:
    K key; 
    V value;
};

// Node构造
template<typename K, typename V>
Node<K, V> :: Node(const K k, const V v, int level) {
    this->key = k;
    this->value = v;
    this->node_level = level;

    // forward 大小为 level + 1
    this->forward = new Node<K, V>*[level + 1];
    // forward 初始化，全都设为0; memset对指针数组初始化后，其值为NULL
    // 如果是对指针变量所指向的内存单元进行清零初始化，那么一定要先对这个指针变量进行初始化，即一定要先让它指向某个有效的地址
    memset(this->forward, 0, sizeof(Node<K, V>*) * (level + 1));
}

// Node析构
template<typename K, typename V>
Node<K, V> :: ~Node() {
    delete []forward; // 析构 forward 内容
}

// 获取key值
template<typename K, typename V>
K Node<K, V> :: get_key() const {
    return key;
}

// 获取value值
template<typename K, typename V>
V Node<K, V> :: get_value() const {
    return value;
}

// 设置value值
template<typename K, typename V>
void Node<K, V> :: set_value(V value) {
    this->value = value;
}


// SkipList 定义
template<typename K, typename V>
class SkipList {
public:
    SkipList(int);
    ~SkipList();
    Node<K, V>* create_node(K, V, int);
    int get_random_level();                
    int insert_element(K, V);               // 增
    void delete_element(K);                 // 删
    int update_element(K, V, bool falg = false);         // 改                    
    bool search_element(K);                 // 查

    void display_list();                    // 打印跳表
    void clear();                           // 清空跳表
    int size();                             // 返回跳表节点数(不包含头节点)

    // 数据落盘和数据加载
    void dump_file();
    void load_file();
    

private:
    void get_key_value_from_string(const std::string& str, std::string* key, std::string* value);
    bool is_valid_string(const std::string& str);

private:
    int _max_level;                         // 跳表层数上限
    int _skiplist_level;                    // 当前跳表的最高层 
    Node<K, V>* _header;                    // 跳表中头节点指针
    int _element_count;                     // 跳表中节点数
    std::ofstream _file_writer;
    std::ifstream _file_reader;

};

// skiplist有参构造
template<typename K, typename V>
SkipList<K, V> :: SkipList(int max_level) {
    this->_max_level = max_level;
    this->_skiplist_level = 0;
    this->_element_count = 0;
    K k;
    V v;
    this->_header = new Node<K, V>(k, v, _max_level);
}

// skiplist析构
template<typename K, typename V>
SkipList<K, V> :: ~SkipList() {
    delete _header;
}

// 创建新节点
template<typename K, typename V>
Node<K, V>* SkipList<K, V> :: create_node(const K k, const V v, int level) {
    Node<K, V> *n = new Node<K, V>(k, v, level);
    return n;
}

// 获得随机层高函数
template<typename K, typename V>
int SkipList<K, V> :: get_random_level() {
    int k = 1;
    while (rand() % 2) {
        k++;
    }
    k = (k < _max_level) ? k : _max_level;              // 最大不超过上限
    return k;
}

// 返回跳表大小
template<typename K, typename V> 
int SkipList<K, V> :: size() {
    return _element_count;
}

// 插入键值对 return 1代表element已存在  return 0 代表插入成功
/* 
                           +------------+
                           |  insert 50 |
                           +------------+
level 4     +-->1+                                                      100
                 |
                 |                      insert +----+
level 3         1+-------->10+---------------> | 50 |          70       100
                                               |    |
                                               |    |
level 2         1          10         30       | 50 |          70       100
                                               |    |
                                               |    |
level 1         1    4     10         30       | 50 |          70       100
                                               |    |
                                               |    |
level 0         1    4   9 10         30   40  | 50 |  60      70       100
                                               +----+

*/
template<typename K, typename V>
int SkipList<K, V>::insert_element(const K key, const V value) {
    
    mtx.lock(); // 插入操作，加锁,
    Node<K, V> *current = this->_header;

    // update 是一个指针数组，数组内存存放指针，指向 node 节点
    Node<K, V> *update[_max_level + 1];
    memset(update, 0, sizeof(Node<K, V>*)*(_max_level + 1));  

    // 从最高层开始遍历
    for(int i = _skiplist_level; i >= 0; i--) {
        // 只要当前节点非空，且 key 小于目标, 就会向后遍历
        while(current->forward[i] != NULL && current->forward[i]->get_key() < key) {
            current = current->forward[i];  // 节点向后移动
        }
        update[i] = current;  // update[i] 记录当前层最后符合要求的节点
    }

    // 遍历到 level 0 说明到达最底层了，forward[0]指向的就是跳跃表下一个邻近节点
    current = current->forward[0];
    // 此时 current->get_key() >= key !!!

    // ① 插入元素已经存在
    if (current != NULL && current->get_key() == key) {
        std::cout << "key : " << key << ", exists" << std::endl;
        mtx.unlock();
        return 1;  // ~ 插入元素已经存在，返回 1
    }

    // ② 如果当前 current 不存在，或者 current->get_key > key
    if (current == NULL || current->get_key() != key ) {
        // 随机生成层的高度，也即 forward[] 大小
        int random_level = get_random_level();

        // 更新 update 数组，原本 [_skip_list_level random_level] 范围内的 NULL 改为 _header
        if (random_level > _skiplist_level) {
            for (int i = _skiplist_level + 1; i < random_level + 1; i++) {
                update[i] = _header;
            }
            _skiplist_level = random_level; // ~ 更新 random_level
        }

        // 创建节点
        Node<K, V>* inserted_node = create_node(key, value, random_level);
        
        // 该操作类似于  new_node->next = pre_node->next; pre_node->next = new_node; 只不过逐层进行
        for (int i = 0; i <= random_level; i++) {
            inserted_node->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = inserted_node;
        }
        std::cout << "Successfully inserted key : " << key << ", value : " << value << std::endl;
        _element_count ++;
    }
    mtx.unlock();
    return 0;  // 成功返回 0
}

// 删除节点
template<typename K, typename V>
void SkipList<K, V> :: delete_element(K key) {
    mtx.lock();
    Node<K, V>* current = this->_header;
    Node<K, V>* update[_max_level];
    memset(update, 0, sizeof(Node<K, V>*) * (_max_level + 1));

    for (int i = _skiplist_level; i >=0; i--) {
        while (current->forward[i] != NULL && current->forward[i]->get_key() < key) {
            current = current->forward[i];
        }
        update[i] = current;
    }

    current = current->forward[0];

    // 非空，且key为目标值
    if (current != NULL && current->forward[0]->get_key() == key) {
        // 从最底层开始删除 update->forward 指向的节点，即目标节点
        for (int i = 0; i <= _skiplist_level; i++) {
            // 如果 update[i] 已经不指向 current 说明 i 的上层也不会指向 current
            if (update[i]->forward[i] != current) {
                break;
            }
            // 删除操作，类似于 node->next = node->next->next
            update[i]->forward[i] = current->forward[i];
        }

        // 重新确定 _skip_list_level 因为可能删除的元素层数恰好是当前跳跃表的最大层数
        while (_skiplist_level > 0 && _header->forward[_skiplist_level] == 0) {
            _skiplist_level--;
        }

        std::cout << "Successfully deleted key : "<< key << std::endl;
        _element_count--;
    }
    else {
        std::cout << key << " is not exist, please check your input !\n";
        mtx.unlock();
        return;
    }
    mtx.unlock();
    return;

}

// Search for element in skip list 
/*
                           +------------+
                           |  select 60 |
                           +------------+
level 4     +-->1+                                                      100
                 |
                 |
level 3         1+-------->10+------------------>50+           70       100
                                                   |
                                                   |
level 2         1          10         30         50|           70       100
                                                   |
                                                   |
level 1         1    4     10         30         50|           70       100
                                                   |
                                                   |
level 0         1    4   9 10         30   40    50+-->60      70       100
*/
// 查询节点
template<typename K, typename V>
bool SkipList<K, V> :: search_element(K key) {
    Node<K, V>* current = _header;
    for (int i = _skiplist_level; i >= 0; i--) {
        while (current->forward[i] != NULL && current->forward[i]->get_key() < key) {
            current = current->forward[i];
        }
    }

    current = current->forward[0];

    if (current && current->get_key() == key) {
        std::cout << "Found key: " << key << ", value: " << current->get_value() << std::endl;
        return true;
    }
    std::cout << "Not Found Key: " << key << std::endl;
    return false;
}

// 更改节点
// 如果当前键存在，更新值
// 如果当前键不存在，通过 flag 指示是否创建该键 (默认false)
// flag = true ：创建 key value
// flag = false : 返回键不存在
// 返回值 ret = 1 表示更新成功 ret = 0 表示创建成功 ret = -1 表示更新失败且创建失败
template<typename K, typename V>
int SkipList<K, V> :: update_element(const K key, const V value, bool flag) {
    mtx1.lock();
    Node<K, V>* current = this->_header;
    Node<K, V>* update[_max_level + 1];
    memset(update, 0, sizeof(Node<K, V>*) * (_max_level + 1));

    for (int i = _skiplist_level; i>= 0; i--) {
        while (current->forward[i] != NULL && current->forward[i]->get_key() < key) {
            current = current->forward[i];
        }
        update[i] = current;
    }

    current = current->forward[0];

    if (current != NULL && current->get_key() == key) {
        std::cout << "key : " << key << ", exists" << std::endl;
        std::cout << "old value : " << current->get_value() << " --> "; // 打印 old value
        current->set_value(value);
        std::cout << "new value : " << current->get_value() << std::endl;
        mtx1.unlock();
        return 1; // 插入元素已经存在，只是修改操作，返回 1
    } else {
        // flag = true 则使用 insert_element 添加
        if (flag) {
             SkipList<K, V>::insert_element(key, value);
            mtx1.unlock();
            return 0;  // 说明 key 不存在，但是创建了它
        } else {
            std::cout << key << " is not exist, please check your input !\n";
            mtx1.unlock();
            return -1; // 表示 key 不存在，并且不被允许创建
        }
    }
}

// 打印跳表
template<typename K,typename V> 
void SkipList<K, V>::display_list() {
     std::cout << "\n *****  Skip List  ***** "<<"\n";
     for (int i = 0; i <= _skiplist_level; i++) {
         Node<K, V>* node = this->_header->forward[i];
         std::cout << "Level " << i << ": ";
         while (node != NULL) {
            std::cout << node->get_key() << " : " << node->get_value() << "; ";
            node = node->forward[i];
         }
         std::cout << std::endl;
     }
}

// 清空跳表
template<typename K, typename V>
void SkipList<K, V> :: clear() {
    std::cout << "clear ..." << std::endl;
    Node<K, V>* node = this->_header->forward[0];
    // 删除节点
    while (node != NULL) {
        Node<K, V>* temp = node;
        node = node->forward[0];
        delete temp;
    }

    // 重新初始化 _header
    for (int i = 0; i <= _max_level; i++) {
        this->_header->forward[i] = 0;
    }
    this->_skiplist_level = 0;
    this->_element_count = 0;

}

// 数据落盘
template<typename K, typename V>
void SkipList<K, V> :: dump_file() {
    std::cout << "dump_file..." << std::endl;
    _file_writer.open(STORE_FILE);
    // 跳表最后一层已经包含了全部信息
    Node<K, V>* node = this->_header->forward[0];
    while (node != NULL) {
        _file_writer << node->get_key() <<  ":" << node->get_value() << "\n"; // 文件写入
        std::cout << node->get_key() << ":" << node->get_value() << ";\n";   // 打印输出
        node = node->forward[0];
    }
    _file_writer.flush();
    _file_writer.close();
    return;
}

// 数据加载
template<typename K, typename V> 
void SkipList<K, V>::load_file() {
    _file_reader.open(STORE_FILE);
    std::cout << "load_file..." << std::endl;
    std::string line;
    std::string* key = new std::string();
    std::string* value = new std::string();
    while (getline(_file_reader, line)) {
        get_key_value_from_string(line, key, value);
        if (key->empty() || value->empty()) 
            continue;
        insert_element(*key, *value);
        std::cout << "key : " << *key << "value : " << *value << std::endl;
    }
    _file_reader.close();
}

template<typename K, typename V>
void SkipList<K, V> :: get_key_value_from_string(const std::string& str, std::string* key, std::string* value) {
    if (!is_valid_string(str)) {
        return;
    }
    *key = str.substr(0, str.find(delimiter)); // 分隔符之前的为 key 
    *value = str.substr(str.find(delimiter) + 1, str.length()); // 分隔符之后的为 value
}

template<typename K, typename V>
bool SkipList<K, V> :: is_valid_string(const std::string& str) {
    if (str.empty()) {
        return false;
    }
    if (str.find(delimiter) == std::string::npos) {
        return false;
    }
    return true;
        
}