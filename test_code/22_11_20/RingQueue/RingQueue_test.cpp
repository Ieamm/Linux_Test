#include"RingQueue.hpp"
#include"task.hpp"

const string operator_set = "+-*/%";

void* producer(void *arg)
{
    RingBlockQueue<task> *p_rbq = static_cast<RingBlockQueue<task>*>(arg);
    //生产任务
    while(true)
    {
        int lhs = rand() % 10;
        int rhs = rand() % 10;
        char op = operator_set[rand() % 5];
        task t(lhs,rhs,op);
        p_rbq->push(t);
        cout << "productor[" << pthread_self() << "]" << (unsigned long)time(nullptr) << "生产了一个任务:"
        << lhs << op << rhs << "=?" << endl;
    }

    return nullptr;
}

void* consumer(void *arg)
{
    RingBlockQueue<task>* p_rbq = static_cast<RingBlockQueue<task>*>(arg);
    //消费任务
    while(true)
    {
        task t = p_rbq->pop();
        int result = t.run();
        int lhs,rhs;
        char op;
        t.get(&lhs,&rhs,&op);
        cout << "consumer[" << pthread_self() << "]" << (unsigned long)time(nullptr) << "消费了一个任务:"
        << lhs << op << rhs << "=" << result << endl;

        sleep(1);//为了体现生产者与消费者之间的同步关系.
    }

    return nullptr;
}

int main()
{
    srand((unsigned long)time(nullptr) ^ getpid());
    
    pthread_t p1,p2,p3,c1,c2,c3;
    RingBlockQueue<task> rbq;
    pthread_create(&p1,nullptr,producer,(void*)&rbq);
    sleep(1);
    pthread_create(&p2,nullptr,producer,(void*)&rbq);
    sleep(1);
    pthread_create(&p3,nullptr,producer,(void*)&rbq);
    sleep(1);
    pthread_create(&c1,nullptr,consumer,(void*)&rbq);
    pthread_create(&c2,nullptr,consumer,(void*)&rbq);
    pthread_create(&c3,nullptr,consumer,(void*)&rbq);


    pthread_join(p1,nullptr);
    pthread_join(p2,nullptr);
    pthread_join(p3,nullptr);
    pthread_join(c1,nullptr);
    pthread_join(c2,nullptr);
    pthread_join(c3,nullptr);
    
    return 0;
}