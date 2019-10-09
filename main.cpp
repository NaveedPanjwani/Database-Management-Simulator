#include <stdio.h>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <iostream>
#include <algorithm>
#include <string.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <cstdlib>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <pthread.h>
using namespace std;

struct block {
	int priority;
	int user;
	int group;
	int position;
	int timestart;
	int timelength;
};
static int otherprioritycounter = 0;
static bool change = false;
static pthread_mutex_t lock;
static int totalrequestgroup1 = 0;
static int totalrequestgroup2 = 0;
static int duetogroup = 0;
static int duetolockedposition = 0;
static int prioritymembers = 0;
static bool positionheld[10] = { false,false,false,false,false,false,false,false,false,false };
static int positionheldnum[10] = { 0,0,0,0,0,0,0,0,0,0 };
static pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
static pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;

void *access_house(void *family_void_ptr){
	pthread_mutex_lock(&lock);
	struct block b = *(struct block *)family_void_ptr;
	cout << "User " << b.user << " from Group " << b.group << " arrives to the DBMS "<<endl;
	if (change == true) {
		if (b.group != b.priority) {
			cout << "User " << b.user << " is waiting due to its group" << endl;
			pthread_cond_wait(&cond1, &lock);
			totalrequestgroup2++;
			duetogroup++;
		}
		else {
			prioritymembers++;
			totalrequestgroup1++;
		}
	}
	if (positionheld[b.position - 1] == true) {
		cout << "User " << b.user << " is waiting : position " << b.position << " of the database is being used by user " << positionheldnum[b.position - 1] << endl;
		duetolockedposition++;
		pthread_cond_wait(&cond2, &lock);
	}
	
	cout << "User " << b.user << " is accessing the position " << b.position << " of the database for " << b.timelength << " second(s) " << endl;
	positionheldnum[b.position - 1] = b.user;
	positionheld[b.position - 1] = true;

	pthread_mutex_unlock(&lock);
	sleep(b.timelength);
	pthread_mutex_lock(&lock);
	cout << "User "<<b.user<<" finished its execution " << endl;
	positionheld[b.position - 1] = false;
	positionheldnum[b.position - 1] = 0;
	if (b.group == b.priority) {
		otherprioritycounter--;
	}
	prioritymembers--;
	if (otherprioritycounter == 0 && b.group==b.priority) {
		pthread_cond_broadcast(&cond1);
		if (b.priority == 1) {
			cout << endl << "All users from Group " << 1 << " finished their execution" << endl << "The users from Group " << 2 << " start their execution " << endl << endl;
		}
		else if (b.priority == 2) {
			cout << endl << "All users from Group " << 2 << " finished their execution" << endl << "The users from Group " << 1 << " start their execution " << endl << endl;
		}
		
	}
	if (otherprioritycounter == 0 && change == false) {
		pthread_cond_broadcast(&cond1);
		if (b.priority == 1) {
			cout << endl << "All users from Group " << 1 << " finished their execution" << endl << "The users from Group " << 2 << " start their execution " << endl << endl;
		}
		else if (b.priority == 2) {
			cout << endl << "All users from Group " << 2 << " finished their execution" << endl << "The users from Group " << 1 << " start their execution " << endl << endl;
		}

	}
	if (positionheld[b.position - 1] == false) {
		pthread_cond_broadcast(&cond2);
	}
	pthread_mutex_unlock(&lock);
	/*
	pthread_mutex_lock(&lock);
	if (positionheld[b.position - 1] = false) {
		//pthread_cond_broadcast(&cond1);
	}
	pthread_mutex_unlock(&lock);
	*/
	return NULL;
}

int main()
{
	int startinggroup;
	int g, po, ts, tl;
	cin >> startinggroup;
	string line;
	struct block b[10];
	int p = 0;
	cout << startinggroup << endl;
	while (cin >> b[p].group >> b[p].position >> b[p].timestart >> b[p].timelength) {
		if (startinggroup == b[p].group) {
			otherprioritycounter++;
		}
		if (otherprioritycounter == 1) {
			change = true;
		}
		b[p].user = p + 1;
		b[p].priority = startinggroup;
		cout << b[p].group << b[p].position << b[p].timestart << b[p].timelength << endl;
		p++;
	}
	pthread_t tid[p];
	pthread_mutex_init(&lock, NULL); // Initialize access to 1

	for (int i = 0; i < p; i++)
	{
		sleep(b[i].timestart);
		if (pthread_create(&tid[i], NULL, access_house, (void *)&b[i]))
		{
			fprintf(stderr, "Error creating thread\n");
			return 1;
		}
	}
	for (int i = 0; i < p; i++) {
		pthread_join(tid[i], NULL);
	}
	cout << endl << "Total Requests: " << endl << "Group 1: " << totalrequestgroup1 << endl << "Group 2: " << totalrequestgroup2 << endl;
	cout <<endl<< "Requests that waited: " << endl << "Due to its group: " << duetogroup << endl << "Due to a locked position: " << duetolockedposition << endl;
	return 0;
}

