#include "BlockQueue.hpp"
#include "task.hpp"

#include <stdlib.h>
#include <ctime>

#include <unistd.h>
#include <sys/types.h>

// 运算符集合
const string operator_set = "+-*/%";

void *Productor(void *arg)
{
    BlockQueue<task> *pbq = (BlockQueue<task> *)arg;
    while (true)
    {
        //创建任务 -- 创建任务并非只有计算任务这一种类型,诸如网络请求,磁盘请求等任务,是极其消耗时间的.
        //因此,在创建任务期间,消费者模型依然可以运行其他任务,这就是"并发"机制的体现.
        task t(rand() % 100, rand() % 100, operator_set[rand() % 5]);
        
        // 放入阻塞队列
        pbq->push(t);
        int lhs,rhs;
        char operate;
        t.get(&lhs,&rhs,&operate);
        cout << "productor[" << pthread_self() << "]" << (unsigned) time(nullptr) << "生产了一个任务:" 
        << lhs << operate << rhs << "= ?" << endl;
        
        //为了体现消费者-生产者之间的同步机制,使生产者放慢,此时若同步成立,消费者就必须跟着生产者的步调执行.
        sleep(1);
    }
}

void *Consumer(void *arg)
{
    BlockQueue<task> *pbq = (BlockQueue<task> *)arg;
    while(true)
    {
        // 获取任务
        task t = pbq->pop();
        
        // 处理任务 -- 处理任务也是需要花费时间的!
        // 在消费者处理任务时,生产者也在生产任务,这同样是"并发"机制的体现.
        int result = t();

        int lhs,rhs;
        char operate;
        t.get(&lhs,&rhs,&operate);
        cout << "consumer[" << pthread_self() << "]" << (unsigned) time(nullptr) << "消费了一个任务:" 
        << lhs << operate << rhs << "=" << result << endl;
    }
}



int main()
{
    // 随机数种子生成
    srand((unsigned long)time(nullptr) ^ getpid());
    pthread_t consumer_tid, product_tid;
    // 阻塞队列---也就是生产-消费者模型中的"交易场所",作为临界资源存在.
    BlockQueue<task> bq;

    pthread_create(&consumer_tid, nullptr, Consumer, (void *)&bq);
    pthread_create(&product_tid, nullptr, Productor, (void *)&bq);

    pthread_join(consumer_tid, nullptr);
    pthread_join(product_tid, nullptr);
    return 0;
}