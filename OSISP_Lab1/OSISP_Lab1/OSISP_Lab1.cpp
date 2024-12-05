#include <iostream>
#include <fstream>
#include <sstream>
#include <Windows.h>
#include <filesystem>
#include <chrono>

int THREAD_COUNT = 1;
bool show_text_chunk;
bool show_only_general_info;
std::string filename = "";
int source_length = 0;
int result_length = 0;

struct thread_args {
	int begin;
	int end;
	int index;

	thread_args(int begin, int end, int index) {
		this->begin = begin;
		this->end = end;
		this->index = index;
	}

	thread_args() {
		this->begin = -1;
		this->end = -1;
		this->index = -1;
	}
};

HANDLE* threads;
DWORD* thread_ids;

thread_args* args_arr;


DWORD WINAPI thread_function(LPVOID lpParam) {
	thread_args* args = (thread_args*)lpParam;

	std::fstream file;
	int length = args->end - args->begin + 1;
	char* text = new char[length + 1];
	file.open(filename);
	file.seekg(args->begin);
	file.read(text, length);
	file.close();
	text[length] = 0;
	std::stringstream ss(text);
	auto str = ss.str();
	int total_size = str.size();
	if (args->end == source_length) {
		str[total_size - 1] = 0;
	}
	if (!show_only_general_info) {
		if (show_text_chunk) {
			printf("[Thread %d - Completed segment from %d to %d - Size: %d Segment: %s]\n", args->index, args->begin, args->end, total_size, str.c_str());
		}
		else {
			printf("[Thread %d - Completed segment from %d to %d - Size: %d]\n", args->index, args->begin, args->end, total_size);
		}
	}
	result_length += total_size;
	delete[] text;
	return 0;
}

void launch_threads() {
	for (int i = 0; i < THREAD_COUNT; i++) {
		threads[i] = CreateThread(NULL, 0, thread_function, &args_arr[i], 0, &thread_ids[i]);
		if (threads[i] == 0) {
			std::cout << "Error creating thread " << i << std::endl;
		}
	}

	WaitForMultipleObjects(THREAD_COUNT, threads, TRUE, INFINITE);

	for (int i = 0; i < THREAD_COUNT; i++) {
		CloseHandle(threads[i]);
	}

	delete[] threads;
	delete[] thread_ids;
	delete[] args_arr;
}

int divide_file_between_threads() {
	std::filesystem::path path = filename;
	int size = 0;
	try {
		size = std::filesystem::file_size(path);
	}
	catch (...) {
		return 1;
	}
	
	source_length = size;
	int i = 0;
	for (; i < THREAD_COUNT; i++) {
		args_arr[i] = thread_args((size / THREAD_COUNT) * i, (size / THREAD_COUNT) * (i + 1) - 1, i);
	}
	args_arr[i - 1].end = size;
	return 0;
}

int input() {
	std::cout << "File name: \n";
	std::cin >> filename;

	std::cout << "Thread count: \n";
	std::cin >> THREAD_COUNT;

	std::string temp_bool_choice;
	std::cout << "Show text chunks[y/n]: \n";

	std::cin >> temp_bool_choice;

	if (temp_bool_choice == "y" || temp_bool_choice == "Y") {
		show_text_chunk = true;
	}
	else {
		show_text_chunk = false;
	}

	if (THREAD_COUNT > 50) {
		THREAD_COUNT = 50;
		std::cout << "Launching max threads of 50" << std::endl;
	}
	else if (THREAD_COUNT <= 0) {
		std::cout << "Not enough threads" << std::endl;
		return 1;
	}

	threads = new HANDLE[THREAD_COUNT];
	thread_ids = new DWORD[THREAD_COUNT];
	args_arr = new thread_args[THREAD_COUNT]{};

	return 0;
}

void benchmark() {
	for (int i = 1; i <= 50; i++) {
		THREAD_COUNT = i;

		threads = new HANDLE[THREAD_COUNT];
		thread_ids = new DWORD[THREAD_COUNT];
		args_arr = new thread_args[THREAD_COUNT]{};

		filename = "data.txt";

		show_only_general_info = true;

		int file_error = divide_file_between_threads();
		if (file_error) {
			printf("![Error launching benchmark case #%d]\n", i);
			continue;
		}

		auto start = std::chrono::high_resolution_clock::now();

		launch_threads();

		auto end = std::chrono::high_resolution_clock::now();

		printf("[Launch #%d finished successfully, %d threads, time: %I64d microseconds]\n", i, i, std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
		printf("\t[Source and result match: %d]\n", (source_length == result_length - 1));

		result_length = 0;
	}
}

//int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
int main()
{
	bool launch_benchmark;
	std::string temp_bool_choice;
	std::cout << "Start benchmark[y/n]: \n";

	std::cin >> temp_bool_choice;

	if (temp_bool_choice == "y" || temp_bool_choice == "Y") {
		launch_benchmark = true;
	}
	else {
		launch_benchmark = false;
	}

	if (launch_benchmark) {
		benchmark();
		return 0;
	}

	int incorrect_input = input();
	if (incorrect_input) {
		return 1;
	}

	int file_error = divide_file_between_threads();
	
	if (file_error) {
		std::cout << "No such file found" << std::endl;
		return 1;
	}

	auto start = std::chrono::high_resolution_clock::now();

	launch_threads();

	auto end = std::chrono::high_resolution_clock::now();
	std::cout << std::chrono::duration_cast<std::chrono::microseconds>(end - start) << std::endl;
	std::cout << "Source and result are equal: ";
	std::cout << (source_length == result_length - 1) << std::endl;
}
