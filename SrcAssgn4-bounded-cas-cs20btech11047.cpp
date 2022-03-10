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

std::atomic<bool> cas_lock;
//static int p=20;


// K 
static int k=0;
time_t my_time = time(NULL);

static int t1=0;
static int t2=0;


static double avg_bwcas=0;


static double worst_bwcas=0;

std::mutex mtx;

std::mutex mtx_avg;

static std::ofstream output("Bounded_CAS_output.txt");

static std::vector<bool> waiting;

auto getsystime()
{
    //time_t my_time = time(NULL);
    return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
}



//This function conatians critical section
void function_bwcas(int i, int p)
{
    auto begin = std::chrono::high_resolution_clock::now();
    bool expect_lock_available=true, try_locking=false;
    waiting.at(p)=true;
    bool key=false;

    //try to acquire lock
    while(waiting.at(p) && key==false)
    {
        key=cas_lock.compare_exchange_strong(expect_lock_available,try_locking);
        expect_lock_available=true;
    }

    //cal of avg and worst
    auto end = std::chrono::high_resolution_clock::now();
     auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
     mtx_avg.lock();
     avg_bwcas=avg_bwcas+(elapsed.count()*(1e-9));
     if( (elapsed.count()*(1e-9))>worst_bwcas )
     {
         worst_bwcas= (elapsed.count()*(1e-9));
     }
     mtx_avg.unlock();

    waiting.at(p)=false;

     //critical section 
      mtx.lock();
     auto timenow2=getsystime();
      output<<i+1<<" th time Thread : "<<std::this_thread::get_id()<<" in critical section at "<<ctime(&timenow2)<<std::endl;
     mtx.unlock();
     //sleep this thread for t1 seconds
     std::this_thread::sleep_for(std::chrono::seconds(t1));
      
     //exit critical section

    int j= (p+1)%waiting.size();
    while ((j!=p) && !waiting.at(j))
    {
        j= (j+1)%waiting.size();
    }

    if(j==i)
    {
        cas_lock=true;
    }
    else
    {
        waiting.at(j)=false;
    }
    
    return ;
}




//This function is assigned to threads
void testCS_bwcas(int x)
{
    std::thread::id t_id = std::this_thread::get_id();
    for(int i=0;i<k;i++)
    {
        mtx.lock();
        auto timenow=getsystime();
         output<<i+1<<" th time Thread : "<<t_id<<" requested CS at "<<ctime(&timenow)<<std::endl;
        mtx.unlock();
        //critical function
        function_bwcas(i,x);

        mtx.lock();
        auto timenow1=getsystime();
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

    std::ofstream report("Bounded_CAS_report_file.txt", std::ios::out | std::ios::app);
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


    


    cas_lock=true;
    output<<"Bounded waiting CAS :"<<std::endl;
    std::vector<std::thread> threads_bwcas;
    for(int i=0;i<n;i++)
    {
        waiting.push_back(true);
    }

    for(int i=0;i<n;i++)
    {
        threads_bwcas.push_back(std::thread(testCS_bwcas,i));

    }

    for(int i=0;i<n;i++)
    {
        threads_bwcas.at(i).join();
    }

    for(int i=0;i<n;i++)
    {
        threads_bwcas.at(i).~thread();
    }
    threads_bwcas.clear();

    

    
    avg_bwcas = avg_bwcas / (n*k);
    std::cout<<"bounded waiting cas Avg : "<<avg_bwcas<<"seconds"<<std::endl<<"Worst case waiting time : "<<worst_bwcas<<std::endl;
    report<<"bounded waiting cas Avg : "<<avg_bwcas<<"seconds"<<std::endl<<"Worst case waiting time : "<<worst_bwcas<<std::endl;

    






    output.close();

    waiting.clear();



    return 0;
}