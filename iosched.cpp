
//necessary import for the program 
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <queue>
#include <deque>
#include <algorithm>
#include <iomanip>  
#include <list>
#include <limits>

//structure that represents a IO request object 
struct IORequest {
    //represents ID of request
    int id;
    //represents arrival time of request
    int arrival_time;
    //represents track destination of request
    int track;
    //trivial intialization of the requests start and end time 
    int start_time = -1;
    int end_time = -1;
    //intialization as we intialize request objects when we read the input files
    //which gives us arrival time and track destination for each requests 
    //and we calculate id as we read each line 
    IORequest(int id, int arrival, int track)
            : id(id), arrival_time(arrival), track(track) {}
};
//base class for the scheduling algorithms 
class IOScheduler {
public:
    //constructor 
    virtual ~IOScheduler() = default;
    //2 necessary methods as from intrsuctions one to add request to scheduler 
    virtual void add_request(IORequest* request) = 0;
    //the other to get the next request from the scheduler 
    virtual IORequest* get_next_request(int current_track) = 0;
};

//subclass that represents the FIFO scheduler
//derived from scheduler superclass
class FIFOScheduler : public IOScheduler {
private:
    //used queue to hold our scheduled io requests 
    std::queue<IORequest*> io_queue;
public:
    //trivial add request mehod that just adds the request
    //to our queue
    void add_request(IORequest* request) override {
        io_queue.push(request);
    }

    //trivial get request method that 
    IORequest* get_next_request(int current_track) override {
        //first checks if empty 
        if (io_queue.empty()) 
        {
            //if it is we return null
            return nullptr;
        }
        //if not we get the request using front since fifo
        IORequest* next_request = io_queue.front();
        //remove request from queue since now we are processing it  
        io_queue.pop();
        //return the request
        return next_request;
    }
};
//subcalss that represents the SSTF scheduler 
//derived from superclass IOscheduler 
class SSTFScheduler : public IOScheduler {
private:
    //using dequeu to hold our scheduled requests
    std::deque<IORequest*> io_queue;
public:
    //again pretty trivial add method to add a request to the dequeue
    void add_request(IORequest* request) override {
        io_queue.push_back(request);
    }
    //gets the next request from our scheduler 
    IORequest* get_next_request(int current_track) override {
        //first checks if empty and if so returns null
        if (io_queue.empty()) 
        {
            return nullptr;
        }
        //if not empty we find the request that is closest to the current track
        //regardless of direction 
        auto closest_it = std::min_element(io_queue.begin(), io_queue.end(),
                                           [current_track](IORequest* a, IORequest* b) {
                                               return std::abs(a->track - current_track) < std::abs(b->track - current_track);
                                           });
        //get that request in another variable so we can erase it from the queue
        IORequest* next_request = *closest_it;
        io_queue.erase(closest_it);
        //return the request
        return next_request;
    }
};
//subclass that represents LOOK scheduler
//derived from scheduler superclass
class LOOKScheduler : public IOScheduler {
private:
    //uses dequeue to hold the scheduled requests 
    std::deque<IORequest*> io_queue;
    //variable to keep track of direction we are currently
    //moving the head in and as mentioned in isntructiosn
    //we always move up first so intialized to 1
    int direction=1;
public:
    //add requests to the dequeue 
    void add_request(IORequest* request) override {
        io_queue.push_back(request);
        //an additional step i took was to keep it sorted based on id else base on track
        //since i realized there were scenarios were requests may have same destination track
        //so to choose the one to go first should be based on id. 
        std::sort(io_queue.begin(), io_queue.end(), [](IORequest* a, IORequest* b) {
            if (a->track == b->track) 
            {
                return a->id < b->id; 
            }
            return a->track < b->track;
        });
    }
    //gets the next request from the dequeue 
   IORequest* get_next_request(int current_track) override {
        //first checks if are queue is empty and if so returns a null ptr
        if (io_queue.empty())
        { 
            return nullptr; 
        }
        //variable to hold our next request 
        IORequest* next_request = nullptr;
        //vairable to keep track of the minimum distance we have seen a request
        //needs to travel from the current track 
        int min_distance = std::numeric_limits<int>::max();
        //variable used to keep track of the id as well since if two are same distance
        //away from current track then id should be used as tie breaker 
        int best_id = std::numeric_limits<int>::max();
        //helper variable to see if we found a request to return sicne if not must 
        //reverse directions 
        bool found_request = false;
        //if we are moving up intially 
        if (direction == 1) {
            //loop through the queue
            for (auto it = io_queue.begin(); it != io_queue.end(); ++it) {
                //check if the requests from queue has track greater or equal to current track
                //since right now we are only moving up 
                if ((*it)->track >= current_track) {
                    //if so calculate distance 
                    int dist = std::abs((*it)->track - current_track);
                    //if we have found a lower distance movement for a request or we found one
                    //that is equal to the min we have seeen so far check if its id is less than 
                    //the one we have seen so far
                    if (dist < min_distance || (dist == min_distance && (*it)->id < best_id)) {
                        //if so we assign next request return vairable to that request from the queue
                        next_request = *it;
                        //assign min distance for comparisons in later iterations 
                        min_distance = dist;
                        //assign our best id similarlry. 
                        best_id = (*it)->id;
                        //we have found a request so need need to check other direction
                        found_request = true;
                    }
                }
            }
            //if we haven't found request then we must start moving down 
            if (!found_request) {
                //so now we move down so direction is changed for reference in all further calls of this
                //method or iterations 
                direction = -1;
                //loop through queue again similarly 
                for (auto it = io_queue.begin(); it != io_queue.end(); ++it) {
                    //calculate distnace using absolute value since now we are moving down
                    //from current track 
                    int dist = std::abs((*it)->track - current_track);
                    //pretty much same comparison check if we have found a request whose track 
                    //is closer to current track or its same distance as a minimum we have found before
                    //but higher priopritized id 
                    if (dist < min_distance || (dist == min_distance && (*it)->id < best_id)) {
                        //set our return varaible
                        next_request = *it;
                        //set our distance helper for further iterations 
                        min_distance = dist;
                        //set our id helper for further iterations
                        best_id = (*it)->id;
                    }
                }
            }
            //if our intial direction was down not up from the last call of this method 
        } else if (direction == -1) {
            //loop through queue again similarly 
            for (auto it = io_queue.begin(); it != io_queue.end(); ++it) {
                //now checking if request has track less than or equal to current track
                if ((*it)->track <= current_track) {
                    //get distance from current track 
                    int dist = std::abs((*it)->track - current_track);
                    //if our distance is smaller than min seen so far or its the same but id of
                    //this request is higher priority 
                    if (dist < min_distance || (dist == min_distance && (*it)->id < best_id)) {
                        //assign our return variable 
                        next_request = *it;
                        //assign distance helper for future iterations 
                        min_distance = dist;
                        //assign ide helper for future iterations 
                        best_id = (*it)->id;
                        //we found a request moving down so no need to reverse directions 
                        found_request = true;
                    }
                }
            }
            //if we weren't able to find one from our intial move down 
            if (!found_request) {
                //change direction to up for future iterations and calls
                //of this method 
                direction = 1;
                //loop through queue 
                for (auto it = io_queue.begin(); it != io_queue.end(); ++it) {
                    //calculate distance from current track 
                    int dist = std::abs((*it)->track - current_track);
                    //again if distance is less than min seen so far or id has higher priority 
                    if (dist < min_distance || (dist == min_distance && (*it)->id < best_id)) {
                        //set our return variable 
                        next_request = *it;
                        //set our distance helper for next iterations 
                        min_distance = dist;
                        //set our id helper for next iterations 
                        best_id = (*it)->id;
                    }
                }
            }
        }
        //remove request from queue 
        io_queue.erase(std::remove(io_queue.begin(), io_queue.end(), next_request), io_queue.end());
        //return requests
        return next_request;
    }
  

};
//subclass that represents the CLOOK scheduler
//derived from IOscheduler superclass
class CLOOKScheduler : public IOScheduler {
private:
    //used dequeu to hold scheduled requests 
    std::deque<IORequest*> io_queue;
public:
    //add requests to the scheduler 
    void add_request(IORequest* request) override {
        io_queue.push_back(request);
        //i have this sort as previous in the LOOK algorithm due to a problem as I was facing
        //when two requests were same distance away from current track or 2 requests have same destination
        //track id should be tie breaker 
        std::sort(io_queue.begin(), io_queue.end(), [](IORequest* a, IORequest* b) {
             if (a->track == b->track) 
            {
                return a->id < b->id; 
            }
            return a->track < b->track;
        });
    }
    //gets the next requests 
    IORequest* get_next_request(int current_track) override {
        //first check if queue is empty and if so reutrns null ptr 
        if (io_queue.empty()) 
            {
                return nullptr;
            }
        //goes through queue and finds the first requests is greater than or equal to current track 
        auto it = std::find_if(io_queue.begin(), io_queue.end(), [current_track](IORequest* r) { return r->track >= current_track; });
        //if none found the request to be returned is just the requests at start of queue 
        if (it == io_queue.end()) it = io_queue.begin(); 
        //set our return variable 
        IORequest* next_request = *it;
        //remove requests from queue 
        io_queue.erase(it);
        //return requests
        return next_request;
    }
};
//subclass that represents the FLOOk scheduler dereived from
//super class IOscheduler 
class FLOOKScheduler : public IOScheduler {
private:
    //now use 2 queues as mentioned in the instructions
    //io_queue represents the active queueu
    std::deque<IORequest*> io_queue;
    //and add queue is same as what was in the instructions 
    std::deque<IORequest*> add_queue;
    //helper to keep track of direction to move head in always starts
    //by moving up so intailized to 1 
    int direction = 1; 
public:
    //add method that adds it but it actually adds it to the add_queue
    //not to the io_queue(active_queue)
    void add_request(IORequest* request) override {
        add_queue.push_back(request);
        //again using sort due to problems i was running into as mentioned with
        //the LOOK and CLOOK scheduler 
        std::sort(add_queue.begin(), add_queue.end(), [](IORequest* a, IORequest* b) {
            if (a->track == b->track) 
            {
                return a->id < b->id; 
            }
            return a->track < b->track;
        });
    }
    //gets the next requests 
    IORequest* get_next_request(int current_track) override {
        //first checks if io_queue is empty since we may need to swap 
        if (io_queue.empty()) 
        {
            //if so before swapping we check if add_queue is empty 
            if (add_queue.empty()) 
                {
                    //if so we reutnr null 
                    return nullptr;
                }
            //if add is not empty that mneans more request have been scheduled 
            //so swap add and io(active)queue. 
            std::swap(io_queue, add_queue);
        }
        //helper variable declaration/intiailization
        //return variable that holds request
        IORequest* next_request = nullptr;
        //tracker varaible to hold min distanc we have seen so far 
        int min_distance = std::numeric_limits<int>::max();
        //tracker variable to hold id of the min distance we have seen so far
        int best_id = std::numeric_limits<int>::max();
        //bool indicator to see if we have found requets moving up 
        bool found_request = false;
        //the main difference between this and LOOk is that for FLOOK the intial direction
        //is always going up from the current track it can never be going down
        //we only go down from current track if we couln't find a request while going up 
        //so loop through queue as we have been ding 
        for (auto it = io_queue.begin(); it != io_queue.end(); ++it) 
        {
            //check that request track is greater than the current since moving up 
            if ((*it)->track >= current_track) 
            {
                //if so get the distance 
                int dist = std::abs((*it)->track - current_track);
                //if the distance is less than min we have seen 
                //or its the same but id of this request has higher priority 
                if (dist < min_distance || (dist == min_distance && (*it)->id < best_id)) 
                {
                    //set our return variable 
                    next_request = *it;
                    //set our distarnce tracker for future iterations
                    min_distance = dist;
                    //set our id tracker for future iterations
                    best_id = (*it)->id;
                    //set ou boolean since now we don't have to try and move down
                    found_request = true;
                }
            }
        }
        //if we were not able to find a request moving up now we move down 
        if (!found_request) {
            //no need to do this really since we alawys move up first then move down
            //with FLOOK but i just added it for consistency
            direction = -1;
            //loop through queue 
            for (auto it = io_queue.begin(); it != io_queue.end(); ++it) 
            {
                //calculate distance 
                int dist = std::abs((*it)->track - current_track);
                //if distance is less than min seen so far or id has higher priotiy 
                if (dist < min_distance || (dist == min_distance && (*it)->id < best_id)) 
                {
                    //set our return varaible
                    next_request = *it;
                    //set our min distance tracker for future iteration
                    min_distance = dist;
                    //set our id tracker for future iterations 
                    best_id = (*it)->id;
                }
            }
        }
        //remove requesut from queue 
        io_queue.erase(std::remove(io_queue.begin(), io_queue.end(), next_request), io_queue.end());
        //return requests from queue
        return next_request;
        
    }
};

void simulate_io_scheduler(IOScheduler& scheduler, const std::vector<IORequest>& requests, bool verbose = false) {
    //helper variable declaration/intialization
    //list to hold our completed requests 
    //many of these used for output 
    std::vector<IORequest> completed_requests;
    //track variable to keep track of current_time 
    int current_time = 0;
    //track variable to keep track of current request
    int current_track = 0;
    //track variable to keep track of active request
    IORequest* active_request = nullptr;
    //index variable for accessing our list of requests that was intiailzied when reading
    //the input file 
    std::size_t request_index = 0;
    //track variable to keep track of total movement of disk head 
    int total_movement = 0;
    //here i essentially tried to follow the pseudocode given in the directions
    //as close as possible to make it simpler 
    //while we have not gone through all requests essentially since thats the only
    //time we will break out of the loop
    while (true) {
        //loop through are requests list and see if any have arrived at current time 
        while (request_index < requests.size() && requests[request_index].arrival_time == current_time) 
        {
            //if they have arrived add it to scheduler to get it ready to be scheduled 
            scheduler.add_request(const_cast<IORequest*>(&requests[request_index]));
            //increment our request index to check if next have arrived or not 
            request_index++;
        }

        //if we have an active request that is now completed after incrementing current time
        if (active_request && active_request->end_time == current_time) 
        {
            //then since its done add it to our list of comeplete requests 
            completed_requests.push_back(*active_request);
            //assign it to null ptr since now no active request so must get a new one 
            active_request = nullptr;

        }
        //if currently now active requests processed 
        if (!active_request) {
            //get the next one from the scheduler 
            active_request = scheduler.get_next_request(current_track);
            if (active_request) 
            {
                //if scheduler returned active request 
                //intialize its start and end time according to our current track and track the requests 
                //wants to get to 
                active_request->start_time = current_time;
                active_request->end_time = current_time + std::abs(active_request->track - current_track);
             
            }
        }
        //this was the only thing that differed from the pseudocode since i was 
        //running into an issue where when we got a request from the scheduler
        //from the previous if statement if it is already at the correct track 
        //then we don't need to do anything 
        if (active_request && active_request->end_time == current_time) 
        {
            //specifically we add it to our list of compelted requests 
            completed_requests.push_back(*active_request);
            //set it to null since now no active request
            active_request = nullptr;
            //then we must continue to next iteration since no need to increment time
            //since nothing was done 
            continue; 

        }
        //if we have an active request 
        if (active_request) {
            //se which direction to move the track accoding to current traack
            //and thhe destination track for the request 
            if (current_track < active_request->track) {
                current_track++;
            } else if (current_track > active_request->track) {
                current_track--;
            }
            //increment our total movement variable 
            total_movement++;
        }
        //if we don't have any active requests and we have reached the end of our
        //request list this implies we have processed all of them so break 
        if (!active_request && request_index == requests.size()) {
            break;
        }
        //increment our current time variable for next iteration 
        current_time++;
    }
    //set the total time once we break the loop since now we have finishde with scheduling 
    int total_time = current_time;
    //declare our helper variables for our ouput 
    double io_utilization = 0;
    double avg_turnaround = 0;
    double avg_wait_time = 0;
    int max_wait_time = 0;
    //go through each request and do appropriate calculations
    //as will be used in sum line for the output 
    for (const auto& req : completed_requests) {
        //relatively trivial computed as given in the instructions 
        io_utilization += (req.end_time - req.start_time);
        avg_turnaround += (req.end_time - req.arrival_time);
        avg_wait_time += (req.start_time - req.arrival_time);
        max_wait_time = std::max(max_wait_time, req.start_time - req.arrival_time);
    }
    //for average must divide by total time or total requests 
    io_utilization /= total_time;
    avg_turnaround /= completed_requests.size();
    avg_wait_time /= completed_requests.size();
    //i had to sort the requests since i maintained a separate list for 
    //requests that finished and since they don't necessarily finish in order
    //i just sorted it by id 
    std::sort(completed_requests.begin(), completed_requests.end(),
          [](const IORequest& a, const IORequest& b) {
              return a.id < b.id;
          });
    //go through each request and print its necessary information as given in
    //the instructions, do spacing requirements as given in directions 
    for (const auto& req : completed_requests) {
        std::cout << std::setw(5) << req.id << ": "
                  << std::setw(5) << req.arrival_time << " "
                  << std::setw(5) << req.start_time << " "
                  << std::setw(5) << req.end_time << std::endl;
    }
    //print out our final sum line for the ouput and apply spacing as given
    //in the requirements 
    std::cout << "SUM: " << total_time << " " << total_movement << " "
              << std::fixed << std::setprecision(4) << io_utilization << " "
              << std::fixed << std::setprecision(2) << avg_turnaround << " "
              << std::fixed << std::setprecision(2) << avg_wait_time << " "
              << max_wait_time<<std::endl;
}
//the main function that will parse input arguments, read the input file, 
//and set up our simulation for the scheduler 
int main(int argc, char* argv[]) {
    //first check if all the necessary arguments are there
    //i am only checking for the necessary ones since I did not implement the 
    //optional arguments 
    if (argc < 3) {
        std::cerr << "Usage: ./iosched -s<scheduler> <inputfile>";
        return 1;
    }
    //create a temp storage variable for out scheculer to be created 
    //and our input file to be read 
    std::string scheduler_arg = argv[1];
    char scheduler_type = scheduler_arg.back(); // Extracts the last character
    std::string input_file = argv[2];
    //essentially read the input file pretty trivial with necessary
    //input parsing 
    std::ifstream infile(input_file);
    //checking if we can open file 
    if (!infile.is_open()) {
        std::cerr << "Error opening file: " << input_file << "";
        return 1;
    }
    //create our list of requests that will be intiailized from reading the
    //inpuut file 
    std::vector<IORequest> requests;
    std::string line;
    //intialize our id variable for our requests objects 
    int id = 0;
    //read through each line 
    while (std::getline(infile, line)) {
        //as said in directions we ignore lines that start with # 
        //or empty lines 
        if (line.empty() || line[0] == '#') continue;
        //read in the lines 
        std::istringstream iss(line);
        //get the 2 parameterts each line for a request object must have 
        int arrival_time, track;
        //if not 2 parameters there is an error 
        if (!(iss >> arrival_time >> track)) {
            std::cerr << "Malformed input line: " << line << "";
            return 1;
        }
        //initialize reequests and add it to the list of our requests
        requests.emplace_back(id++, arrival_time, track);
    }
    //close the file 
    infile.close();
    //declare our scheduler object and intitialize to our argument
    //that we read in before hand
    IOScheduler* scheduler = nullptr;
    //using switch case again as we have learned much more optimal than if-else
    switch(scheduler_type)
    {
        //N is for FIFO
        case 'N':
            scheduler= new FIFOScheduler();
            break;
        //S is for SSTF
        case 'S':
            scheduler= new SSTFScheduler();
            break;
        //L is for LOOK     
        case 'L':
            scheduler= new LOOKScheduler();
            break;
        //C is for CLOOK
        case 'C':
            scheduler= new CLOOKScheduler();
            break;
        //F is for FLOOK
        case 'F':
            scheduler= new FLOOKScheduler();
            break;
        //if none of the cases reached than error since these are the only
        //schedulers we are considering 
        default:
            std::cerr << "Invalid scheduler type: " << scheduler_type << std::endl;
            return 1;
    }
    //start our simulation
    simulate_io_scheduler(*scheduler, requests);
    //delete the scheduler 
    delete scheduler;
    //trivial return statement 
    return 0;
}