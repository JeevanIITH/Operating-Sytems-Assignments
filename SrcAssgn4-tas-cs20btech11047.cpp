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



//flag for keeping lock on which TAS is performed
std::atomic_flag flag = ATOMIC_FLAG_INIT;



// K 
static int k=0;
time_t my_time = time(NULL);


// t1 and t2
static int t1=0;
static int t2=0;


//for calculating avg of TAS
static double avg_tas=0;

//cal for worst time
static double worst_tas=0;


//mutex loc for writing to output
std::mutex mtx;

//for avg calculation
std::mutex mtx_avg;


//output file
static std::ofstream output("TAS_output.txt");


//get system time
auto getsystime()
{
    //time_t my_time = time(NULL);
    return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
}


//this function contains critical section
void function(int i)
{
    //begin clock
    auto begin = std::chrono::high_resolution_clock::now();
    //try to acquire lock
     while(flag.test_and_set())
     {}
     
     //calculation of avg waiting and worst times
     auto end = std::chrono::high_resolution_clock::now();
     auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
     mtx_avg.lock();
     avg_tas=avg_tas+(elapsed.count()*(1e-9));
     if((elapsed.count()*(1e-9))>worst_tas)
     {
         worst_tas=elapsed.count()*(1e-9);
     }
     mtx_avg.unlock();


     //critical section //

      //writing to output file
      mtx.lock();
     auto timenow2=getsystime();
    output<<i+1<<" th time Thread : "<<std::this_thread::get_id()<<" in critical section at "<<ctime(&timenow2)<<std::endl;
     mtx.unlock();
     //sleep this thread for t1 seconds
     std::this_thread::sleep_for(std::chrono::seconds(t1));
      
     //exit critical section
     //release lock
     flag.clear();
     return ;
}



//this function is passed to thread which inturn call critical function
void testCS()
{
    
    std::thread::id t_id = std::this_thread::get_id();
    for(int i=0;i<k;i++)
    {
        //write to output file
        mtx.lock();
        auto timenow=getsystime();
        //std::cout<<i<<"th time Thread : "<<t_id<<" requested CS at "<<ctime(&my_time)<<std::endl;
        output<<i+1<<" th time Thread : "<<t_id<<" requested CS at "<<ctime(&timenow)<<std::endl;
        mtx.unlock();


        //critical function
        function(i);

        //write to outputfile
        mtx.lock();
        auto timenow1=getsystime();
        //std::cout<<i<<"th time Thread : "<<t_id<<" exit CS at "<<ctime(&my_time)<<std::endl;
        output<<i+1<<" th time Thread : "<<t_id<<" exit CS at "<<ctime(&timenow1)<<std::endl;
        mtx.unlock();
        //sleep this thread
        std::this_thread::sleep_for(std::chrono::seconds(t2));
    }
}



int main()
{
    std::cout<<"input file name is inp-params.txt"<<std::endl<<" sample input file must contains format like  5 3 1 2 : numbers are seperated by spaces NOT with commas "<<std::endl;
    std::string file_name="inp-params.txt";

    std::ofstream report("TAS_report_file.txt", std::ios::out | std::ios::app);
    std::string n_str,k_str,t1_str,t2_str;
    int n=0;

    //read input file
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


    output<<"TAS ME output :"<<std::endl;
    std::vector<std::thread> threads;
    for(int i=0; i<n ;i++)
    {
        threads.push_back(std::thread(testCS));
    }
    //time variable 
    
    
    for(int i=0;i<n;i++)
    {
        threads.at(i).join();
    }

    for(int i=0;i<n;i++)
    {
        threads.at(i).~thread();
    }
    threads.clear();


    

    avg_tas= avg_tas/(n*k);

    

    std::cout<<"tas Avg : "<<avg_tas<<"seconds"<<std::endl<< "Worst case waiting time : "<<worst_tas<<std::endl;
    report<<"tas Avg : "<<avg_tas<<"seconds"<<std::endl<< "Worst case waiting time : "<<worst_tas<<std::endl;
    
    






    output.close();

    



    return 0;
}