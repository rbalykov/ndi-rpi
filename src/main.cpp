
#include "video_monitor.h"
#include <csignal>
#include <thread>
#include <iostream>

static bool program_run = true;

void sigterm_callback(int s)
{
	(void) s;
	program_run = false;
}

int main (int argc, char ** argv)
{
	(void) argc;
	(void) argv;

//	unsigned int n = std::thread::hardware_concurrency();

	std::signal(SIGTERM, sigterm_callback);
	std::signal(SIGINT, sigterm_callback);

	video_monitor m(&program_run);
	return m.run();
}

