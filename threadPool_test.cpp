#include"threadPool.hpp"
#include<memory>
#include"task.hpp"
int main()
{
    srand((unsigned long)time(nullptr) ^ pthread_self());

    const string operatorSet = "+-*/%";
    //使用智能指针管理内存
    unique_ptr<threadPool<task>>up_tp(threadPool<task>::getInstance());
    cout << up_tp->getInstance() << endl;
    cout << "启动线程池" << endl;
    up_tp->start();

    while(true)
    {
        int one_p = rand() % 10;
        int two_p = rand() % 10;
        char op = operatorSet[rand()%5];
        cout << "主线程派发计算任务:"<< one_p << op << two_p << "=?" << endl;
        task t(one_p,two_p,op);
        up_tp->push(t);
        
        sleep(1);
    }
    
    return 0;
}