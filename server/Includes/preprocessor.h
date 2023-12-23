#define FIELDLENGTH 127
#define SERVERLOGSX 50
#define SERVERLOGSY 0
#define ARGVMAX 100
#define LINESIZE 1024
#include <ncurses.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <netdb.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "auxFuncs.h"
#include "nodes.h"
#include "comparator.h"
#include "hasher.h"
#include "doublelistcomp.h"
#include "doublelist.h"
#include "stackList.h"
#include "queueList.h"
#include "dliterator.h"
#include "atomicOps.h"
#include "hashtablecomp.h"
#include "htiterator.h"
#include "server.h"
#include "cmdstruct.h"
#include "hashtablecmd.h"
#include "../cmdModules/Includes/servercmds.h"


