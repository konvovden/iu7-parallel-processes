#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>
#include <omp.h>

#define CONTROLLER_RANK 0
#define SEGMENTS_PER_NODE 10

#ifdef _OPENMP
#define OPENMP_THREADS omp_get_max_threads()
#endif

#ifndef _OPENMP
#define OPENMP_THREADS 1
#endif


#define SEND_PERIMETER_TAG 0
#define SEND_INTERVAL_TAG 1
#define STOP_TAG 2

#define RANDOM_RECTANGLES_COUNT 1000000
#define BORDER_RANGE 10000
#define MAX_RECTANGLE_LENGTH 100

typedef struct {
	int min_x;
	int min_y;
	int max_x;
	int max_y;
} Rectangle;

void generate_example(int example_number, Rectangle** rectangles, int* length);

int calculate_perimeter(Rectangle* rectangles, int rectangles_length);

typedef struct {
	int pos;
	int type; // 0 - begin, 1 - ending
	int min;
	int max;
	int rectangle_index;
} Slice;

int calculate_perimeter_for_interval(Rectangle* rectangles, int rectangles_length, int from, int to, int max_x);
int calculate_perimeter_for_slices(Slice* slices, int slices_length);
int increase_interval_uses(int* interval, int min_interval, int max_interval, int start, int end);
int decrease_interval_uses(int* interval, int min_interval, int max_interval, int start, int end);
int count_different_intervals(int* interval, int length);

void sync_entry_point(int example);
void controller_entry_point(int example);
void worker_entry_point(int example);

int myrank, nprocs;

int main(int argc, char** argv)
{
	int example = 4;

	int len;
	char name[MPI_MAX_PROCESSOR_NAME];

	MPI_Init(&argc, &argv);

	MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
	MPI_Get_processor_name(name, &len);

	printf("Hello from host %s[%d] %d of %d\n", name, len, myrank, nprocs);

	if (nprocs == 1)
	{
		sync_entry_point(example);
	}
	else
	{
		if (myrank == CONTROLLER_RANK)
		{
			controller_entry_point(example);
		}
		else
		{
			worker_entry_point(example);
		}
	}

	MPI_Finalize();

	return 0;
}

#pragma region Entry Points

void sync_entry_point(int example)
{
	printf("[%d] nprocs = %d, running synchronously...\n", myrank, nprocs);

	Rectangle* rectangles = NULL;
	int rectangles_length;

	generate_example(example, &rectangles, &rectangles_length);

	int start_time = MPI_Wtime();

	int perimeter = calculate_perimeter(rectangles, rectangles_length);

	printf("[%d] Total perimeter: %i\n", myrank, perimeter);
	printf("[%d] Calculation time: %lf\n", myrank, MPI_Wtime() - start_time);

	free(rectangles);
}

void controller_entry_point(int example)
{
	printf("[%d] Running as controller...\n", myrank);

	Rectangle* rectangles = NULL;
	int rectangles_length;

	generate_example(example, &rectangles, &rectangles_length);

	int start_time = MPI_Wtime();

	printf("[%d] Sending %d rectangles to workers\n", myrank, rectangles_length);
	MPI_Bcast(rectangles, rectangles_length * 4, MPI_INT, CONTROLLER_RANK, MPI_COMM_WORLD);

	int min_x = INT_MAX;
	int max_x = INT_MIN;

	for (int i = 0; i < rectangles_length; i++)
	{
		if (rectangles[i].min_x < min_x)
			min_x = rectangles[i].min_x;

		if (rectangles[i].max_x > max_x)
			max_x = rectangles[i].max_x;
	}

	int perimeter = 0;

	int segments = SEGMENTS_PER_NODE * (nprocs - 1);
	double real_segment_length = abs(max_x - min_x) * 1.0 / segments;
	int int_segment_length = abs(max_x - min_x) / segments;

	if (int_segment_length < 1)
	{
		int_segment_length = 1;
		segments = abs(max_x - min_x);
	}

	int current_x = min_x;


	for (int i = 0; i < segments + (nprocs - 1); i++)
	{
		MPI_Status status;
		int worker_perimeter = -1;

		MPI_Recv(&worker_perimeter, 1, MPI_INT, MPI_ANY_SOURCE, SEND_PERIMETER_TAG, MPI_COMM_WORLD, &status);
		printf("[%d] Received perimeter %d from worker %d\n", myrank, worker_perimeter, status.MPI_SOURCE);

		if (worker_perimeter != -1)
			perimeter += worker_perimeter;

		int* buf = (int*)malloc(2 * sizeof(int));

		if (i < segments)
		{
			int next_x = current_x + int_segment_length;
			double must_be = (min_x + real_segment_length * (i + 1));
			if (next_x < must_be)
				next_x += must_be - next_x;

			buf[0] = current_x;
			buf[1] = next_x;

			printf("[%d] Sending interval from %d to %d to worker %d\n", myrank, current_x, next_x, status.MPI_SOURCE);
			MPI_Send(buf, 2, MPI_INT, status.MPI_SOURCE, SEND_INTERVAL_TAG, MPI_COMM_WORLD);

			current_x = next_x;
		}
		else
		{
			printf("[%d] No intervals for worker %d. Sending stop tag\n", myrank, status.MPI_SOURCE);

			MPI_Send(buf, 2, MPI_INT, status.MPI_SOURCE, STOP_TAG, MPI_COMM_WORLD);
		}
	}

	printf("[%d] Total perimeter: %i\n", myrank, perimeter);
	printf("[%d] Calculation time: %lf\n", myrank, MPI_Wtime() - start_time);

	free(rectangles);
}

void worker_entry_point(int example)
{
	printf("[%d] Running as worker...\n", myrank);

	int rectangles_length = get_rectangles_length_for_example(example);
	Rectangle* rectangles = (Rectangle*)malloc(sizeof(Rectangle) * rectangles_length);

	MPI_Bcast(rectangles, rectangles_length * 4, MPI_INT, CONTROLLER_RANK, MPI_COMM_WORLD);

	printf("[%d] Received %d rectangles from controller\n", myrank, rectangles_length);

	int max_x = INT_MIN;

	for (int i = 0; i < rectangles_length; i++)
	{
		if (rectangles[i].max_x > max_x)
			max_x = rectangles[i].max_x;
	}

	int current_perimeter = -1;
	int current_from = -1;
	int current_to = -1;

	while(1)
	{

		printf("[%d] Sending perimeter %d for interval from %d to %d to controller\n", myrank, current_perimeter, current_from, current_to);

		MPI_Send(&current_perimeter, 1, MPI_INT, CONTROLLER_RANK, SEND_PERIMETER_TAG, MPI_COMM_WORLD);

		int* interval = (int*)malloc(2 * sizeof(int));
		MPI_Status status;

		MPI_Recv(interval, 2, MPI_INT, CONTROLLER_RANK, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

		if (status.MPI_TAG == SEND_INTERVAL_TAG)
		{
			current_from = interval[0];
			current_to = interval[1];

			printf("[%d] Received interval from %d to %d from controller\n", myrank, current_from, current_to);

			int segments = OPENMP_THREADS;

			if(myrank == 0)
				segments = OPENMP_THREADS - 1;

			double real_segment_length = abs(current_to - current_from) * 1.0 / segments;
			int int_segment_length = abs(current_to - current_from) / segments;

			if (int_segment_length < 1)
			{
				int_segment_length = 1;
				segments = abs(current_to - current_from);
			}

			int* segments_borders = (int*)malloc(segments * 2 * sizeof(int));

			int current_x = current_from;

			for (int i = 0; i < segments; i++)
			{
				int next_x = current_x + int_segment_length;
				double must_be = (current_from + real_segment_length * (i + 1));
				if (next_x < must_be)
					next_x += must_be - next_x;

				segments_borders[i * 2] = current_x;
				segments_borders[i * 2 + 1] = next_x;

				current_x = next_x;
			}

			current_perimeter = 0;

#pragma omp parallel for
			for (int i = 0; i < segments; i++) {
				//printf("%d/%d\n",omp_get_thread_num(), omp_get_max_threads());
				current_perimeter += calculate_perimeter_for_interval(rectangles, rectangles_length, segments_borders[i * 2], segments_borders[i * 2 + 1], max_x);
			}

			free(segments_borders);
		}
		else if (status.MPI_TAG == STOP_TAG)
		{
			printf("[%d] Received stop tag from controller\n", myrank);
			break;
		}
	}

	free(rectangles);
}

#pragma endregion

#pragma region Examples

int get_rectangles_length_for_example(int exampleNumber)
{
	int length = 0;

	switch (exampleNumber)
	{
	case 1:
		length = 5;
		break;
	case 2:
		length = 30;
		break;
	case 3:
		length = 32;
		break;
	case 4:
		length = RANDOM_RECTANGLES_COUNT;
		break;
	}


	return length;
}

void generate_example(int exampleNumber, Rectangle** rectangles, int* length)
{
	*length = get_rectangles_length_for_example(exampleNumber);
	*rectangles = (Rectangle*)malloc(*length * sizeof(Rectangle));
	Rectangle* ptr = *rectangles;

	switch (exampleNumber)
	{
	case 1: // 76
	{
		ptr[0].min_x = 0;
		ptr[0].min_y = 10;
		ptr[0].max_x = 10;
		ptr[0].max_y = 15;

		ptr[1].min_x = 5;
		ptr[1].min_y = 13;
		ptr[1].max_x = 17;
		ptr[1].max_y = 18;

		ptr[2].min_x = 8;
		ptr[2].min_y = 6;
		ptr[2].max_x = 14;
		ptr[2].max_y = 11;

		ptr[3].min_x = 11;
		ptr[3].min_y = 16;
		ptr[3].max_x = 12;
		ptr[3].max_y = 17;

		ptr[4].min_x = 13;
		ptr[4].min_y = 4;
		ptr[4].max_x = 15;
		ptr[4].max_y = 20;

		break;
	}
	case 2: // 624
	{
		ptr[0].min_x = -33;
		ptr[0].min_y = 29;
		ptr[0].max_x = -15;
		ptr[0].max_y = 33;

		ptr[1].min_x = -24;
		ptr[1].min_y = 21;
		ptr[1].max_x = -15;
		ptr[1].max_y = 29;

		ptr[2].min_x = -17;
		ptr[2].min_y = 23;
		ptr[2].max_x = 2;
		ptr[2].max_y = 26;

		ptr[3].min_x = -9;
		ptr[3].min_y = 20;
		ptr[3].max_x = -4;
		ptr[3].max_y = 28;

		ptr[4].min_x = 7;
		ptr[4].min_y = 21;
		ptr[4].max_x = 15;
		ptr[4].max_y = 31;

		ptr[5].min_x = 5;
		ptr[5].min_y = 23;
		ptr[5].max_x = 13;
		ptr[5].max_y = 26;

		ptr[6].min_x = 8;
		ptr[6].min_y = 24;
		ptr[6].max_x = 11;
		ptr[6].max_y = 25;

		ptr[7].min_x = -27;
		ptr[7].min_y = 5;
		ptr[7].max_x = -21;
		ptr[7].max_y = 19;

		ptr[8].min_x = -22;
		ptr[8].min_y = 13;
		ptr[8].max_x = -11;
		ptr[8].max_y = 17;

		ptr[9].min_x = -11;
		ptr[9].min_y = 8;
		ptr[9].max_x = 2;
		ptr[9].max_y = 15;

		ptr[10].min_x = -7;
		ptr[10].min_y = 11;
		ptr[10].max_x = -3;
		ptr[10].max_y = 13;

		ptr[11].min_x = 2;
		ptr[11].min_y = 15;
		ptr[11].max_x = 9;
		ptr[11].max_y = 20;

		ptr[12].min_x = 9;
		ptr[12].min_y = 9;
		ptr[12].max_x = 12;
		ptr[12].max_y = 20;

		ptr[13].min_x = -5;
		ptr[13].min_y = 4;
		ptr[13].max_x = 13;
		ptr[13].max_y = 9;

		ptr[14].min_x = 6;
		ptr[14].min_y = -5;
		ptr[14].max_x = 12;
		ptr[14].max_y = 2;

		ptr[15].min_x = -12;
		ptr[15].min_y = -1;
		ptr[15].max_x = -2;
		ptr[15].max_y = 5;

		ptr[16].min_x = -17;
		ptr[16].min_y = 3;
		ptr[16].max_x = -10;
		ptr[16].max_y = 11;

		ptr[17].min_x = -31;
		ptr[17].min_y = 3;
		ptr[17].max_x = -17;
		ptr[17].max_y = 5;

		ptr[18].min_x = -27;
		ptr[18].min_y = 0;
		ptr[18].max_x = -19;
		ptr[18].max_y = 3;

		ptr[19].min_x = -30;
		ptr[19].min_y = -3;
		ptr[19].max_x = -29;
		ptr[19].max_y = -2;

		ptr[20].min_x = -28;
		ptr[20].min_y = -11;
		ptr[20].max_x = -22;
		ptr[20].max_y = -4;

		ptr[21].min_x = -22;
		ptr[21].min_y = -13;
		ptr[21].max_x = -18;
		ptr[21].max_y = -9;

		ptr[22].min_x = -18;
		ptr[22].min_y = -11;
		ptr[22].max_x = -11;
		ptr[22].max_y = -2;

		ptr[23].min_x = -28;
		ptr[23].min_y = -19;
		ptr[23].max_x = -11;
		ptr[23].max_y = -13;

		ptr[24].min_x = -7;
		ptr[24].min_y = -18;
		ptr[24].max_x = 5;
		ptr[24].max_y = -16;

		ptr[25].min_x = -4;
		ptr[25].min_y = -10;
		ptr[25].max_x = 1;
		ptr[25].max_y = -5;

		ptr[26].min_x = -5;
		ptr[26].min_y = -14;
		ptr[26].max_x = 2;
		ptr[26].max_y = -2;

		ptr[27].min_x = 1;
		ptr[27].min_y = -10;
		ptr[27].max_x = 8;
		ptr[27].max_y = -4;

		ptr[28].min_x = 10;
		ptr[28].min_y = -15;
		ptr[28].max_x = 27;
		ptr[28].max_y = -10;

		ptr[29].min_x = 12;
		ptr[29].min_y = -17;
		ptr[29].max_x = 23;
		ptr[29].max_y = -8;

		break;
	}
	case 3: // 642
	{
		ptr[0].min_x = -33;
		ptr[0].min_y = 29;
		ptr[0].max_x = -15;
		ptr[0].max_y = 33;

		ptr[1].min_x = -24;
		ptr[1].min_y = 21;
		ptr[1].max_x = -15;
		ptr[1].max_y = 29;

		ptr[2].min_x = -17;
		ptr[2].min_y = 23;
		ptr[2].max_x = 2;
		ptr[2].max_y = 26;

		ptr[3].min_x = -9;
		ptr[3].min_y = 20;
		ptr[3].max_x = -4;
		ptr[3].max_y = 28;

		ptr[4].min_x = 7;
		ptr[4].min_y = 21;
		ptr[4].max_x = 15;
		ptr[4].max_y = 31;

		ptr[5].min_x = 5;
		ptr[5].min_y = 23;
		ptr[5].max_x = 13;
		ptr[5].max_y = 26;

		ptr[6].min_x = 8;
		ptr[6].min_y = 24;
		ptr[6].max_x = 11;
		ptr[6].max_y = 25;

		ptr[7].min_x = -27;
		ptr[7].min_y = 5;
		ptr[7].max_x = -21;
		ptr[7].max_y = 19;

		ptr[8].min_x = -22;
		ptr[8].min_y = 13;
		ptr[8].max_x = -11;
		ptr[8].max_y = 17;

		ptr[9].min_x = -11;
		ptr[9].min_y = 8;
		ptr[9].max_x = 2;
		ptr[9].max_y = 15;

		ptr[10].min_x = -7;
		ptr[10].min_y = 11;
		ptr[10].max_x = -3;
		ptr[10].max_y = 13;

		ptr[11].min_x = 2;
		ptr[11].min_y = 15;
		ptr[11].max_x = 9;
		ptr[11].max_y = 20;

		ptr[12].min_x = 9;
		ptr[12].min_y = 9;
		ptr[12].max_x = 12;
		ptr[12].max_y = 20;

		ptr[13].min_x = -5;
		ptr[13].min_y = 4;
		ptr[13].max_x = 13;
		ptr[13].max_y = 9;

		ptr[14].min_x = 6;
		ptr[14].min_y = -5;
		ptr[14].max_x = 12;
		ptr[14].max_y = 2;

		ptr[15].min_x = -12;
		ptr[15].min_y = -1;
		ptr[15].max_x = -2;
		ptr[15].max_y = 5;

		ptr[16].min_x = -17;
		ptr[16].min_y = 3;
		ptr[16].max_x = -10;
		ptr[16].max_y = 11;

		ptr[17].min_x = -31;
		ptr[17].min_y = 3;
		ptr[17].max_x = -17;
		ptr[17].max_y = 5;

		ptr[18].min_x = -27;
		ptr[18].min_y = 0;
		ptr[18].max_x = -19;
		ptr[18].max_y = 3;

		ptr[19].min_x = -30;
		ptr[19].min_y = -3;
		ptr[19].max_x = -29;
		ptr[19].max_y = -2;

		ptr[20].min_x = -28;
		ptr[20].min_y = -11;
		ptr[20].max_x = -22;
		ptr[20].max_y = -4;

		ptr[21].min_x = -22;
		ptr[21].min_y = -13;
		ptr[21].max_x = -18;
		ptr[21].max_y = -9;

		ptr[22].min_x = -18;
		ptr[22].min_y = -11;
		ptr[22].max_x = -11;
		ptr[22].max_y = -2;

		ptr[23].min_x = -28;
		ptr[23].min_y = -19;
		ptr[23].max_x = -11;
		ptr[23].max_y = -13;

		ptr[24].min_x = -7;
		ptr[24].min_y = -18;
		ptr[24].max_x = 5;
		ptr[24].max_y = -16;

		ptr[25].min_x = -4;
		ptr[25].min_y = -10;
		ptr[25].max_x = 1;
		ptr[25].max_y = -5;

		ptr[26].min_x = -5;
		ptr[26].min_y = -14;
		ptr[26].max_x = 2;
		ptr[26].max_y = -2;

		ptr[27].min_x = 1;
		ptr[27].min_y = -10;
		ptr[27].max_x = 8;
		ptr[27].max_y = -4;

		ptr[28].min_x = 10;
		ptr[28].min_y = -15;
		ptr[28].max_x = 27;
		ptr[28].max_y = -10;

		ptr[29].min_x = 12;
		ptr[29].min_y = -17;
		ptr[29].max_x = 23;
		ptr[29].max_y = -8;

		ptr[30].min_x = -21;
		ptr[30].min_y = 24;
		ptr[30].max_x = -1;
		ptr[30].max_y = 41;

		ptr[31].min_x = -29;
		ptr[31].min_y = 25;
		ptr[31].max_x = -3;
		ptr[31].max_y = 44;

		break;
	}
	case 4:
	{
		int min_border = -BORDER_RANGE / 2;
		int max_border = BORDER_RANGE / 2;

		for (int i = 0; i < *length; i++)
		{
			ptr[i].min_x = min_border + rand() % (max_border - min_border);
			ptr[i].min_y = min_border + rand() % (max_border - min_border);
			ptr[i].max_x = ptr[i].min_x + 1 + rand() % MAX_RECTANGLE_LENGTH;
			ptr[i].max_y = ptr[i].min_y + 1 + rand() % MAX_RECTANGLE_LENGTH;

			//printf("%d - %d %d %d %d\n", i, ptr[i].min_x, ptr[i].min_y, ptr[i].max_x, ptr[i].max_y);
		}
		break;
	}
	}
}

#pragma endregion


#pragma region Calculation


int calculate_perimeter(Rectangle* rectangles, int rectangles_length)
{
	int min_x = INT_MAX;
	int max_x = INT_MIN;
	int segments = 4;

	for (int i = 0; i < rectangles_length; i++)
	{
		if (rectangles[i].min_x < min_x)
			min_x = rectangles[i].min_x;

		if (rectangles[i].max_x > max_x)
			max_x = rectangles[i].max_x;
	}

	int perimeter = 0;

	double real_segment_length = abs(max_x - min_x) * 1.0 / segments;
	int int_segment_length = abs(max_x - min_x) / segments;

	if (int_segment_length < 1)
	{
		int_segment_length = 1;
		segments = abs(max_x - min_x);
	}

	int* segment_borders = (int*)malloc(sizeof(int) * segments * 2);

	int current_x = min_x;

	for (int i = 0; i < segments; i++)
	{
		int next_x = current_x + int_segment_length;
		double must_be = (min_x + real_segment_length * (i + 1));
		if (next_x < must_be)
			next_x += must_be - next_x;

		segment_borders[i * 2] = current_x;
		segment_borders[i * 2 + 1] = next_x;


		current_x = next_x;
	}

#pragma omp parallel for
	for (int i = 0; i < segments; i++) {
		//printf("%d/%d\n",omp_get_thread_num(), omp_get_max_threads());
		perimeter += calculate_perimeter_for_interval(rectangles, rectangles_length, segment_borders[i * 2], segment_borders[i * 2 + 1], max_x);
	}

	return perimeter;
}

int compare_rectangle_slices(const void* a, const void* b)
{
	Slice* slice_a = (Slice*)a;
	Slice* slice_b = (Slice*)b;

	if (slice_a->pos < slice_b->pos)
		return -1;
	if (slice_a->pos > slice_b->pos)
		return 1;

	if (slice_a->type == slice_b->type)
		return 0;

	if (slice_a->type == 1)
		return 1;

	return -1;
}

int calculate_perimeter_for_interval(Rectangle* rectangles, int rectangles_length, int from, int to, int max_x)
{
	printf("[%d] Calculating perimeter from %d to %d...\n", myrank, from, to);

	int min_min = INT_MAX;
	int max_max = INT_MIN;


	for (int i = 0; i < rectangles_length; i++)
	{
		if (rectangles[i].min_y < min_min)
			min_min = rectangles[i].min_y;

		if (rectangles[i].max_y > max_max)
			max_max = rectangles[i].max_y;
	}

	int intervals_length = max_max - min_min;
	int* intervals_uses = (int*)malloc(intervals_length * sizeof(int));

	for (int i = 0; i < intervals_length; i++)
		intervals_uses[i] = 0;

	for (int i = 0; i < rectangles_length; i++)
	{
		if (rectangles[i].min_x < from && rectangles[i].max_x >= from)
			increase_interval_uses(intervals_uses, min_min, max_max, rectangles[i].min_y, rectangles[i].max_y);
	}

	Slice* slices = (Slice*)malloc(rectangles_length * 2 * sizeof(Slice));
	int slices_count = 0;

	for (int i = 0; i < rectangles_length; i++)
	{
		if (rectangles[i].min_x >= from && rectangles[i].min_x < to)
		{
			slices[slices_count].rectangle_index = i;
			slices[slices_count].type = 0;
			slices[slices_count].pos = rectangles[i].min_x;
			slices[slices_count].min = rectangles[i].min_y;
			slices[slices_count].max = rectangles[i].max_y;

			slices_count++;
		}

		if (rectangles[i].max_x >= from && rectangles[i].max_x < to || to == max_x && to == rectangles[i].max_x)
		{
			slices[slices_count].rectangle_index = i;
			slices[slices_count].type = 1;
			slices[slices_count].pos = rectangles[i].max_x;
			slices[slices_count].min = rectangles[i].min_y;
			slices[slices_count].max = rectangles[i].max_y;

			slices_count++;
		}
	}

	int perimeter = 0;

	if (slices_count > 0)
	{
		qsort(slices, slices_count, sizeof(Slice), compare_rectangle_slices);


		int current_x = slices[0].pos;

		if (current_x > from)
		{
			int perimeter_by_x = count_different_intervals(intervals_uses, intervals_length) * 2 * (current_x - from);
			perimeter += perimeter_by_x;

			//printf("Added %d to perimeter before first Slice pos %d\n", perimeter_by_x, current_x);
		}

		for (int i = 0; i < slices_count; i++)
		{
			int perimeter_by_x = count_different_intervals(intervals_uses, intervals_length) * 2 * (slices[i].pos - current_x);

			current_x = slices[i].pos;

			int perimeter_by_y;

			if (slices[i].type == 0)
				perimeter_by_y = increase_interval_uses(intervals_uses, min_min, max_max, slices[i].min, slices[i].max);
			else
				perimeter_by_y = decrease_interval_uses(intervals_uses, min_min, max_max, slices[i].min, slices[i].max);

			//printf("Slice[%d] - rectangle %d - type %d - pos %d - from %d to %d - added %d + %d to perimeter (%d)\n", i, slices[i].rectangle_index + 1, slices[i].type, slices[i].pos, slices[i].min, slices[i].max, perimeter_by_x, perimeter_by_y, perimeter);

			perimeter += perimeter_by_x + perimeter_by_y;
		}

		if (current_x < to)
		{
			int perimeter_by_x = count_different_intervals(intervals_uses, intervals_length) * 2 * (to - current_x);
			perimeter += perimeter_by_x;

			//printf("Added %d to perimeter after last Slice pos %d\n", perimeter_by_x, current_x);
		}
	}
	else
	{
		int perimeter_by_x = count_different_intervals(intervals_uses, intervals_length) * 2 * (to - from);
		perimeter += perimeter_by_x;

		//printf("No slices on interval from %d to %d. Added %d to perimeter for existing rectangles\n", from, to, perimeter_by_x);
	}



	//printf("Total perimeter from %d to %d - %d\n", from, to, perimeter);

	return perimeter;
}

int calculate_perimeter_for_slices(Slice* slices, int slices_length)
{
	int perimeter = 0;

	int min_min = INT_MAX;
	int max_max = INT_MIN;

	for (int i = 0; i < slices_length; i++)
	{
		if (slices[i].min < min_min)
			min_min = slices[i].min;

		if (slices[i].max > max_max)
			max_max = slices[i].max;
	}

	qsort(slices, slices_length, sizeof(Slice), compare_rectangle_slices);

	int interval_uses_length = max_max - min_min;
	int* interval_uses = (int*)malloc(interval_uses_length * sizeof(int));

	for (int i = 0; i < interval_uses_length; i++)
		interval_uses[i] = 0;

	int current_x = slices[0].pos;

	for (int i = 0; i < slices_length; i++)
	{
		int perimeter_by_x = count_different_intervals(interval_uses, interval_uses_length) * 2 * (slices[i].pos - current_x);

		current_x = slices[i].pos;

		int perimeter_by_y;

		if (slices[i].type == 0)
			perimeter_by_y = increase_interval_uses(interval_uses, min_min, max_max, slices[i].min, slices[i].max);
		else
			perimeter_by_y = decrease_interval_uses(interval_uses, min_min, max_max, slices[i].min, slices[i].max);

		printf("Slice[%d] - rectangle %d - type %d - pos %d - from %d to %d - added %d + %d to perimeter (%d)\n", i, slices[i].rectangle_index + 1, slices[i].type, slices[i].pos, slices[i].min, slices[i].max, perimeter_by_x, perimeter_by_y, perimeter);

		perimeter += perimeter_by_x + perimeter_by_y;
	}

	free(interval_uses);

	return perimeter;
}

int increase_interval_uses(int* interval, int min_interval, int max_interval, int start, int end)
{
	int added_length = 0;

	for (int j = start - min_interval; j < end - min_interval; j++) {
		if (interval[j] == 0)
			added_length++;

		interval[j]++;
	}

	return added_length;
}

int count_different_intervals(int* interval, int length)
{
	int intervals_count = 0;
	int has_interval = 0;

	for (int i = 0; i < length; i++)
	{
		if (interval[i] > 0 && has_interval == 0)
		{
			has_interval = 1;
			intervals_count++;
		}
		else if (interval[i] == 0 && has_interval == 1)
		{
			has_interval = 0;
		}
	}

	return intervals_count;
}

int decrease_interval_uses(int* interval, int min_interval, int max_interval, int start, int end)
{
	int added_length = 0;

	for (int j = start - min_interval; j < end - min_interval; j++) {
		if (interval[j] == 1)
			added_length++;

		interval[j]--;
	}

	return added_length;
}
#pragma endregion

