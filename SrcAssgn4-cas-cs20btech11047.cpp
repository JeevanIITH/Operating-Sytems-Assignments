 #include <iostream>
 #include <thread>
 #include <string>
 #include <atomic>
 #include <vector>
 #include <sstream>
 #include <fmt/format.h>
 #include <sys/time.h>
 #include <ctime>
 #include <stdlib.h>
 #include <chrono>
 #include <mutex>
 #include <fstream>

 #include <bits/stdc++.h>
 #include <sstream>



std::atomic_flag flag_cas = ATOMIC_FLAG_INIT;


//cas lock
std::atomic<bool> cas_lock;



// K 
static int k=0;
time_t my_time = time(NULL);


//t1 and t2
static int t1=0;
static int t2=0;



//calculate avg
static double avg_cas=0;


//cal worst waiting 
static double worst_cas=0;


//mutex lock
std::mutex mtx;

std::mutex mtx_avg;


//out put file
static std::ofstream output("CAS_output.txt");



auto getsystime()
{
   
    return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
}


//this function conatins critical section 
void function_cas(int i)
{
    auto begin = std::chrono::high_resolution_clock::now();
    bool expect_lock_available=true, try_lock=false;
    //try to acquire lock
     while(!cas_lock.compare_exchange_strong(expect_lock_available,try_lock))
     { expect_lock_available=true;}
     
     //calculate avg time and worst time
     auto end = std::chrono::high_resolution_clock::now();
     auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
     mtx_avg.lock();
     avg_cas=avg_cas+(elapsed.count()*(1e-9));
     if( (elapsed.count()*(1e-9))>worst_cas )
     {
         worst_cas=(elapsed.count()*(1e-9));
     }
     mtx_avg.unlock();

     //critical section 
     mtx.lock();
     auto timenow2=getsystime();
      output<<i+1<<" th time Thread : "<<std::this_thread::get_id()<<" in critical section at "<<ctime(&timenow2)<<std::endl;
     mtx.unlock();
     //sleep this thread for t1 seconds
     std::this_thread::sleep_for(std::chrono::seconds(t1));
      
     //exit critical section
     //release lock
     cas_lock=true;
     return ;
}


//this function is assigned to threads
void testCS_cas()
{
    
    std::thread::id t_id = std::this_thread::get_id();
    for(int i=0;i<k;i++)
    {
        mtx.lock();
        auto timenow=getsystime();
        //std::cout<<i<<"th time Thread : "<<t_id<<" requested CS at "<<ctime(&my_time)<<std::endl;
        output<<i+1<<" th time Thread : "<<t_id<<" requested CS at "<<ctime(&timenow)<<std::endl;
        mtx.unlock();
        
        //critical function
        function_cas(i);

        mtx.lock();
        auto timenow1=getsystime();
        //std::cout<<i<<"th time Thread : "<<t_id<<" exit CS at "<<ctime(&my_time)<<std::endl;
        output<<i+1<<" th time Thread : "<<t_id<<" exit CS at "<<ctime(&timenow1)<<std::endl;
        mtx.unlock();
        //
        std::this_thread::sleep_for(std::chrono::seconds(t2));
    }
}




int main()
{
    std::cout<<"input file name is inp-params.txt"<<std::endl<<" sample input file must contains format like  5 3 1 2 : numbers are seperated by spaces NOT with commas "<<std::endl;
    std::string file_name="inp-params.txt";

    std::ofstream report("CAS_report_file.txt", std::ios::out | std::ios::app);
    std::string n_str,k_str,t1_str,t2_str;
    int n=0;

    std::ifstream infile(file_name);
    if(infile.is_open())
    {
        std::string line;
        std::getline(infile,line);
        std::stringstream ss(line);
        std::getline(ss,n_str,' '); n= std::stoi(n_str);
        std::getline(ss,k_str,' '); k= std::stoi(k_str);
        std::getline(ss,t1_str,' '); t1 = std::stoi(t1_str);
        std::getline(ss,t2_str,' '); t2 =std::stoi(t2_str);

        std::cout<<"Read values are :"<<std::endl<<"N : "<<n<<", K : "<<k<<", t1 : "<<t1<<", t2 : "<<t2<<std::endl;
        report<<"Read values are :"<<std::endl<<"N : "<<n<<", K : "<<k<<", t1 : "<<t1<<", t2 : "<<t2<<std::endl;
        
    }
    else
    {
        std::cout<<"Error in file opening"<<std::endl;
        return -1;
    }




    

    output.clear();


    


    output<<"CAS ME output :"<<std::endl;
    cas_lock=true;
    std::vector<std::thread> threads_cas;
    for(int i=0;i<n;i++)
    {
        threads_cas.push_back(std::thread(testCS_cas));
    }

    for (int i = 0; i < n; i++)
    {
        threads_cas.at(i).join();
    }
    for(int i=0;i<n;i++)
    {
        threads_cas.at(i).~thread();
    }
    threads_cas.clear();



    

    

    

    avg_cas =avg_cas/(n*k);

    std::cout<<"cas Avg : "<<avg_cas<<"seconds"<<std::endl<<"Worst case waiting time : "<<worst_cas<<std::endl;
    report<<"cas Avg : "<<avg_cas<<"seconds"<<std::endl<<"Worst case waiting time : "<<worst_cas<<std::endl;

    






    output.close();

    

    return 0;
}