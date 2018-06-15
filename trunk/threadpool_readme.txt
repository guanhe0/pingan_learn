class CThread 
{ 
private: 
    int          m_ErrCode; 		//错误码
    Semaphore    m_ThreadSemaphore;  //信号量the inner semaphore, which is used to realize 
    unsigned     long m_ThreadID;   //线程id
    bool         m_Detach;       //分离特性 The thread is detached 
    bool         m_CreateSuspended;  //创建即分if suspend after creating 
    char*        m_ThreadName; //线程名字
    ThreadState m_ThreadState;      //线程状态the state of the thread 
protected: 
    void     SetErrcode(int errcode){m_ErrCode = errcode;} //设置错误码
    static void* ThreadFunction(void*); //线程函数
public: 
    CThread(); 
    CThread(bool createsuspended,bool detach); 
    virtual ~CThread(); 
    virtual void Run(void) = 0; //线程执行函数，必须实现
    void     SetThreadState(ThreadState state){m_ThreadState = state;} //设置线程状态

    bool     Terminate(void);    //终止线程Terminate the threa 
    bool     Start(void);        //Start to execute the thread 
    void     Exit(void); 
    bool     Wakeup(void); 
   
    ThreadState  GetThreadState(void){return m_ThreadState;} 
    int      GetLastError(void){return m_ErrCode;} 
    void     SetThreadName(char* thrname){strcpy(m_ThreadName,thrname);} 
    char*    GetThreadName(void){return m_ThreadName;} 
    int      GetThreadID(void){return m_ThreadID;} 

    bool     SetPriority(int priority); 
    int      GetPriority(void); 
    int      GetConcurrency(void); 
    void     SetConcurrency(int num); 
    bool     Detach(void); 
    bool     Join(void); 
    bool     Yield(void); 
    int      Self(void); 
};
CThread-->CWorkerThread
		  CWorkerThread:CThreadPool 
