#include <iostream>
#include <chrono>
#include <cstdlib>
#include <pthread.h>
#include <time.h>
#include <fstream>
#include "../skiplist.h"
//#define STORE_FILE_STRESS_TSET "stress_test/test_result"
#define STORE_FILE_STRESS_TSET "stress_test/test_result.txt"

#define NUM_THREADS 1
#define TEST_COUNT 100
SkipList<int, std::string> skipList(18);

// 在创建过程中一定要保证编写的线程函数与规定的函数指针类型一致：void *(*start_routine) (void *):
void *insertElement(void* threadid) {
    long tid; 
    tid = (long)threadid;
    // std::cout << tid << std::endl;  
    int tmp = TEST_COUNT / NUM_THREADS; // 一个线程写入的数据数
	for (int i = tid * tmp, count = 0; count < tmp; i++) {
        count++;
		skipList.insert_element(rand() % TEST_COUNT, "a"); // 随机产生key为0-1000000随机数， value为a
	}
    pthread_exit(NULL);
}

void *getElement(void* threadid) {
    long tid; 
    tid = (long)threadid;
    // std::cout << tid << std::endl;  
    int tmp = TEST_COUNT / NUM_THREADS; 
	for (int i = tid * tmp, count = 0; count < tmp; i++) {
        count++;
		skipList.search_element(rand() % TEST_COUNT); 
	}
    pthread_exit(NULL);
}

void *deleteElement(void* threadid) {
    long tid; 
    tid = (long)threadid;
    // std::cout << tid << std::endl;  
    int tmp = TEST_COUNT / NUM_THREADS; 
	for (int i = tid * tmp, count = 0; count < tmp; i++) {
        count++;
		skipList.delete_element(rand() % TEST_COUNT); 
	}
    pthread_exit(NULL);
}

void *fixElement(void* threadid) {
    long tid; 
    tid = (long)threadid;
    // std::cout << tid << std::endl;  
    int tmp = TEST_COUNT / NUM_THREADS; 
	for (int i = tid * tmp, count = 0; count < tmp; i++) {
        count++;
		skipList.update_element(rand() % TEST_COUNT, "b"); 
	}
    pthread_exit(NULL);
}


/*
得到当前线程的线程 ID
    pthread_t pthread_self(void);

int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg);
    thread: 传出参数，是无符号长整形数，线程创建成功，会将线程 ID 写入到这个指针指向的内存中
    attr: 线程的属性，一般情况下使用默认属性即可，写 NULL
    start_routine: 函数指针，创建出的子线程的处理动作，也就是该函数在子线程中执行。
    arg: 作为实参传递到 start_routine 指针指向的函数内部
    返回值：线程创建成功返回 0，创建失败返回对应的错误号


int pthread_join(pthread_t thread, void **retval);
    这是一个阻塞函数, 子线程在运行这个函数就阻塞
    子线程退出, 函数解除阻塞, 回收对应的子线程资源, 类似于回收进程使用的函数 wait()
    thread: 要被回收的子线程的线程 ID
    retval: 二级指针，指向一级指针的地址，是一个传出参数，这个地址中存储了 pthread_exit () 传递出的数据，如果不需要这个参数，可以指定为 NULL
    返回值：线程回收成功返回 0，回收失败返回错误号。

*/


int main() {
    //srand(time(NULL));  
    std::ofstream _file_writer;
    // ios::app 追加模式打开文件夹， 以ios::app打开,如果没有文件，那么生成空文件；如果有文件，那么在文件尾 追加
    _file_writer.open(STORE_FILE_STRESS_TSET, std::ios::app);
    _file_writer << "TEST_COUNT : " << TEST_COUNT << " NUM_THREADS: " << NUM_THREADS << std::endl;
    _file_writer.close();
    
    #if 1
    // ~ insert 测试
    {

        pthread_t threads[NUM_THREADS];
        int rc;
        long i;

        auto start = std::chrono::high_resolution_clock::now();

        for( i = 0; i < NUM_THREADS; i++ ) {
            // std::cout << "main() : creating thread, " << i << std::endl;
            rc = pthread_create(&threads[i], NULL, insertElement, (void *)i);

            if (rc) {
                // std::cout << "Error:unable to create thread," << rc << std::endl;
                exit(-1);
            }
        }

        void *ret;
        for( i = 0; i < NUM_THREADS; i++ ) {
            if (pthread_join(threads[i], &ret) !=0 )  {
                // perror("pthread_create() error"); 
                exit(3);
            }
        }
        auto finish = std::chrono::high_resolution_clock::now(); 
        std::chrono::duration<double> elapsed = finish - start;
        _file_writer.open(STORE_FILE_STRESS_TSET, std::ios::app);
        _file_writer << "insert elapsed: " << elapsed.count() << std::endl;
        std::cout<<elapsed.count()<<std::endl;
        _file_writer.close();
    }
    #endif
    // skipList.displayList();
    #if 0
    // ~ update 测试
    {
        pthread_t threads[NUM_THREADS];
        int rc;
        long i;
        auto start = std::chrono::high_resolution_clock::now();

        for( i = 0; i < NUM_THREADS; i++ ) {
            // std::cout << "main() : creating thread, " << i << std::endl;
            rc = pthread_create(&threads[i], NULL, fixElement, (void *)i);

            if (rc) {
                // std::cout << "Error:unable to create thread," << rc << std::endl;
                exit(-1);
            }
        }

        void *ret;
        for( i = 0; i < NUM_THREADS; i++ ) {
            if (pthread_join(threads[i], &ret) !=0 )  {
                // perror("pthread_create() error"); 
                exit(3);
            }
        }

        auto finish = std::chrono::high_resolution_clock::now(); 
        std::chrono::duration<double> elapsed = finish - start;
        _file_writer.open(STORE_FILE_STRESS_TSET, std::ios::app);
        _file_writer << "update elapsed: " << elapsed.count() << std::endl;
        _file_writer.close();
    }
    
    // ~ search 测试
    {
        pthread_t threads[NUM_THREADS];
        int rc;
        long i;
        auto start = std::chrono::high_resolution_clock::now();

        for( i = 0; i < NUM_THREADS; i++ ) {
            // std::cout << "main() : creating thread, " << i << std::endl;
            rc = pthread_create(&threads[i], NULL, getElement, (void *)i);

            if (rc) {
                // std::cout << "Error:unable to create thread," << rc << std::endl;
                exit(-1);
            }
        }

        void *ret;
        for( i = 0; i < NUM_THREADS; i++ ) {
            if (pthread_join(threads[i], &ret) !=0 )  {
                // perror("pthread_create() error"); 
                exit(3);
            }
        }

        auto finish = std::chrono::high_resolution_clock::now(); 
        std::chrono::duration<double> elapsed = finish - start;
        _file_writer.open(STORE_FILE_STRESS_TSET, std::ios::app);
        _file_writer << "get elapsed: " << elapsed.count() << std::endl;
        _file_writer.close();
    }
    
    // ~ delete 测试
    {
        pthread_t threads[NUM_THREADS];
        int rc;
        long i;
        auto start = std::chrono::high_resolution_clock::now();

        for( i = 0; i < NUM_THREADS; i++ ) {
            // std::cout << "main() : creating thread, " << i << std::endl;
            rc = pthread_create(&threads[i], NULL, deleteElement, (void *)i);

            if (rc) {
                // std::cout << "Error:unable to create thread," << rc << std::endl;
                exit(-1);
            }
        }

        void *ret;
        for( i = 0; i < NUM_THREADS; i++ ) {
            if (pthread_join(threads[i], &ret) !=0 )  {
                // perror("pthread_create() error"); 
                exit(3);
            }
        }

        auto finish = std::chrono::high_resolution_clock::now(); 
        std::chrono::duration<double> elapsed = finish - start;
        _file_writer.open(STORE_FILE_STRESS_TSET, std::ios::app);
        _file_writer << "delete elapsed: " << elapsed.count() << std::endl;
        _file_writer << std::endl;
        _file_writer.close();
    }
    #endif


	pthread_exit(NULL);
    
    return 0;

}
