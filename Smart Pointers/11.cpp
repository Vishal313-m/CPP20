//Implement Shared Ptr


#include<iostream>
using namespace std;

template<typename T>
class sharedptr
{


  private:
        T* res;
        int* counter;
       
       void incrementcounter()
       { 
           if(counter){
             (*counter)++; 
             }
       }

       void decrementcounter()
       {
          
          if(counter){
            (*counter)--;
    

    /*If we do a substraction what if countrer value is zero ,Suppose two object is pointing to the same resource ,
    one object is started pointing to someone else ,now only one object remaining , this also went so its counter become zero  so what will happen to that particular resource, it should get deleted */
            if((*counter)==0){     //  
                if(res){
                    delete res;
                    delete counter;
                    res = nullptr;
                    counter = nullptr;
                }
            }
          }
       }

  public:
       //default constructor 
       shared(T*ptr = nullptr): res(ptr), counter(new int(1))
       {
             cout<<"Ctor was called\n";
       }


    // copy Constructor 
       sharedptr<T>(const shared<T> &ptr)
       {
          res= ptr.res;
          counter = ptr.counter;
          incrementcounter();
       }

   //Copy assignment operator 

   sharedptr<T>& operator=(const sharedptr<T>& ptr){

     if(this!=&ptr){
        decrementcounter();
        res=ptr.res;
        counter = ptr.counter;
        
     }
     return *this;
   }

    // Move Constructor 
       sharedptr<T>(shared<T>&&ptr)
       {
          res= ptr.res;
          counter = ptr.counter;
          ptr.res = nullptr;
          ptr.counter = nullptr;

       }
 
       
       //Move assignment operator 

   sharedptr<T>& operator=(const sharedptr<T>&& ptr){

     if(this!=&ptr){
        decrementcounter();
        res=ptr.res;
        counter = ptr.counter;
         ptr.res = nullptr;
         ptr.counter = nullptr;
     }
     return *this;
   }

   
   void reset(T*ptr){
    decrementcounter();
    res = ptr;
    counter = new int(1);
   }


   int get_count(){
    
     if(counter){
        return (*counter);
     }
     return -1;
   }

   T* operator->(){
    return res;
   }

   T& operator*(){
    return (*res);
   }
  
    T* get(){
         return res;
    }

    ~sharedptr(){
        decrementcounter();
    }
}


int main()
{
    sharedptr<int> ptr1; //defualt ctor 

    shared<int> ptr2(new int(10)); //parameterized ctor 

    sharedptr<int> ptr3(ptr2);  // Copy Constructor 

    ptr3 = ptr2;   // Copy Assignment Operator 

    sharedptr<int> ptr4(std::move(ptr1)); // Move Copy Constructor 

    ptr2 = std::move(ptr3);     // Move Copy Assignment Operator 


   ptr1.reset();

   ptr1.reset(new int(6));

   cout<<(*ptr1);  // Implementing *operator 

   ptr1->func(); // Implementing -> operator 

   ptr1.get();  // It will give a raw pointer 

   ptr1.get_count(); // Number of objects pointing to the same resource 

   //dtor
}


-------------------------------------------------------------------------------



#include <iostream>
using namespace std;

template <typename T>
class SharedPointer {
private:
    T* ptr;           // raw pointer
    int* refCount;    // reference counter

public:
    // Constructor
    explicit SharedPointer(T* p = nullptr) : ptr(p), refCount(new int(1)) {
        cout << "Constructor called, count = " << *refCount << "\n";
    }

    // Destructor
    ~SharedPointer() {
        release();
    }

    // Copy constructor
    SharedPointer(const SharedPointer& other) {
        ptr = other.ptr;
        refCount = other.refCount;
        ++(*refCount);  // increase count
        cout << "Copy constructor called, count = " << *refCount << "\n";
    }

    // Copy assignment
    SharedPointer& operator=(const SharedPointer& other) {
        if (this != &other) {
            release(); // release current resource
            ptr = other.ptr;
            refCount = other.refCount;
            ++(*refCount);
            cout << "Copy assignment called, count = " << *refCount << "\n";
        }
        return *this;
    }

    // Move constructor
    SharedPointer(SharedPointer&& other) noexcept {
        ptr = other.ptr;
        refCount = other.refCount;
        other.ptr = nullptr;
        other.refCount = nullptr;
        cout << "Move constructor called\n";
    }

    // Move assignment
    SharedPointer& operator=(SharedPointer&& other) noexcept {
        if (this != &other) {
            release(); // release current resource
            ptr = other.ptr;
            refCount = other.refCount;
            other.ptr = nullptr;
            other.refCount = nullptr;
            cout << "Move assignment called\n";
        }
        return *this;
    }

    // Dereference operators
    T& operator*() { return *ptr; }
    T* operator->() { return ptr; }

    // Get current reference count
    int use_count() const {
        return (refCount ? *refCount : 0);
    }

private:
    // Helper to release resource
    void release() {
        if (refCount) {
            --(*refCount);
            cout << "Release called, count = " << *refCount << "\n";
            if (*refCount == 0) {
                delete ptr;
                delete refCount;
                cout << "Resource freed\n";
            }
        }
    }
};

// Test class
class Demo {
public:
    Demo() { cout << "Demo created\n"; }
    ~Demo() { cout << "Demo destroyed\n"; }
    void hello() { cout << "Hello from Demo!\n"; }
};

int main() {
    cout << "=== Copy Example ===\n";
    SharedPointer<Demo> sp1(new Demo());
    SharedPointer<Demo> sp2 = sp1;  // copy constructor
    SharedPointer<Demo> sp3;
    sp3 = sp1;                      // copy assignment
    cout << "sp1 count = " << sp1.use_count() << "\n";

    cout << "\n=== Move Example ===\n";
    SharedPointer<Demo> sp4 = std::move(sp1);   // move constructor
    SharedPointer<Demo> sp5(new Demo());
    sp5 = std::move(sp4);                       // move assignment
    cout << "sp5 count = " << sp5.use_count() << "\n";

    cout << "\n=== End of main ===\n";
    return 0;
}