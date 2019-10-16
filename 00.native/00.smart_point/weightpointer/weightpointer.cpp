#include <stdio.h>
#include <utils/RefBase.h>
 
#define INITIAL_STRONG_VALUE (1<<28)
 
using namespace android;
 
class WeightClass : public RefBase
{
public:
        void printRefCount()
        {
                int32_t strong = getStrongCount();
                weakref_type* ref = getWeakRefs();
 
                printf("-----------------------\n");
                printf("Strong Ref Count: %d.\n", (strong  == INITIAL_STRONG_VALUE ? 0 : strong));
                printf("Weak Ref Count: %d.\n", ref->getWeakCount());
                printf("-----------------------\n");
        }
};
 
class StrongClass : public WeightClass
{
public:
        StrongClass()
        {
                printf("Construct StrongClass Object.\n");
        }
 
        virtual ~StrongClass()
        {
                printf("Destory StrongClass Object.\n");
        }
};
 
 
class WeakClass : public WeightClass
{
public:
        WeakClass()
        {
                extendObjectLifetime(OBJECT_LIFETIME_WEAK);
 
                printf("Construct WeakClass Object.\n");
        }
 
        virtual ~WeakClass()
        {
                printf("Destory WeakClass Object.\n");
        }
};
 
class ForeverClass : public WeightClass
{
public:
        ForeverClass()
        {
                extendObjectLifetime(OBJECT_LIFETIME_WEAK);
 
                printf("Construct ForeverClass Object.\n");
        }
 
        virtual ~ForeverClass()
        {
                printf("Destory ForeverClass Object.\n");
        }
};
 
 
void TestStrongClass(StrongClass* pStrongClass)
{
    printf("pStrongClass=%p\n", pStrongClass);
        wp<StrongClass> wpOut = pStrongClass; // 此时　弱引用计数为:1 强引用计数为:0 
        pStrongClass->printRefCount();
 
        {
                sp<StrongClass> spInner = pStrongClass;// 此时　弱应用计数为:2 强引用计数为:1
                pStrongClass->printRefCount();
                // 由于此时强引用计数为:1 当出大括号时,pStrongClass 就会被释放掉　
        }
        // 从此时开始　就不能操作　pStrongClass 指针了
        printf("pStrongClass=%p\n", pStrongClass);
        sp<StrongClass> spOut = wpOut.promote();
        printf("spOut: %p.\n", spOut.get());
}
 
void TestWeakClass(WeakClass* pWeakClass)
{
        wp<WeakClass> wpOut = pWeakClass;// WeakRefCount:1 StrongRefCount:0 
        pWeakClass->printRefCount();
 
        {
                sp<WeakClass> spInner = pWeakClass; // WeakRefCount:2 StrongRefCount:1 
                pWeakClass->printRefCount();
        }
 
        pWeakClass->printRefCount(); // WeakRefCount:1 StrongRefCount:0 
        sp<WeakClass> spOut = wpOut.promote();
        pWeakClass->printRefCount(); // WeakRefCount:2 StrongRefCount:1 

        printf("spOut: %p.\n", spOut.get());

        // return 时：　释放 spOut,　此时　WeakRefCount:1 StrongRefCount:0 
        // 由于此时　WeakRefCount:1,　此时释放　wpOut，　pWeakClass所指向的内容被释放
}
 
 
void TestForeverClass(ForeverClass* pForeverClass)
{
        wp<ForeverClass> wpOut = pForeverClass; // WeakRefCount:1 StrongRefCount:0 
        pForeverClass->printRefCount(); 
 
        {
                sp<ForeverClass> spInner = pForeverClass; // WeakRefCount:2 StrongRefCount:1 
                pForeverClass->printRefCount();
        }
        pForeverClass->printRefCount(); // WeakRefCount:1 StrongRefCount:0 

        sp<ForeverClass> spOut = wpOut.promote();
        pForeverClass->printRefCount(); // WeakRefCount:2 StrongRefCount:1 
        printf("spOut: %p.\n", spOut.get());
}
 
int main(int argc, char** argv)
{
        printf("Test Strong Class: \n");
        StrongClass* pStrongClass = new StrongClass();
        pStrongClass->printRefCount();
        TestStrongClass(pStrongClass);
        // pStrongClass->printRefCount();　// 由于pStrongClass　在　TestStrongClass　中被释放掉了，所以这里会段错误
 
        printf("\n\n\n\nTest Weak Class: \n");
        WeakClass* pWeakClass = new WeakClass();
        pWeakClass->printRefCount();
        TestWeakClass(pWeakClass);
        // pWeakClass->printRefCount();　// 这里会报段错误
 
        printf("\n\n\n\nTest Froever Class: \n");
        ForeverClass* pForeverClass = new ForeverClass();
        TestForeverClass(pForeverClass);
        // pForeverClass->printRefCount();
        // delete pForeverClass;

        printf("pStrongClass=%p pWeakClass=%p pForeverClass=%p\n", pStrongClass, pWeakClass, pForeverClass);
 
        return 0;
}




#if 0
// android 5 下的定义
	enum {
		OBJECT_LIFETIME_WEAK    = 0x0001,
		OBJECT_LIFETIME_FOREVER = 0x0003
	};

// android 7 中的定义
    enum {
        OBJECT_LIFETIME_STRONG  = 0x0000,
        OBJECT_LIFETIME_WEAK    = 0x0001,
        OBJECT_LIFETIME_MASK    = 0x0001
    };

/*
    总结：
        1. 在android7 下就没有　OBJECT_LIFETIME_FOREVER，　所以，就做不了　TestForeverClass　测试
        2. android7 中的　OBJECT_LIFETIME_WEAK　== OBJECT_LIFETIME_MASK, 他俩的释放，都是根据　弱指针来判断是否此刻要释放
*/
#endif