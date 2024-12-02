#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

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

int calculate_perimeter_for_slices(Slice* slices, int slices_length);
int increase_interval_uses(int* interval, int min_interval, int max_interval, int start, int end);
int decrease_interval_uses(int* interval, int min_interval, int max_interval, int start, int end);

int main()
{
	Rectangle* rectangles = NULL;
	int rectangles_length;

	generate_example(2, &rectangles, &rectangles_length);

	int perimeter = calculate_perimeter(rectangles, rectangles_length);

	printf("Total perimeter: %i\n", perimeter);

	free(rectangles);

	return 0;
}

#pragma region Examples

void generate_example(int exampleNumber, Rectangle** rectangles, int* length)
{
	switch (exampleNumber)
	{
	case 1:
	{
		*length = 5;

		*rectangles = (Rectangle*)malloc(*length * sizeof(Rectangle));
		Rectangle* ptr = *rectangles;

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
	case 2:
	{
		*length = 30;
		*rectangles = (Rectangle*)malloc(*length * sizeof(Rectangle));
		Rectangle* ptr = *rectangles;

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
	}
}

#pragma endregion


#pragma region Calculation

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

int calculate_perimeter(Rectangle* rectangles, int rectangles_length)
{
	int rectangle_events_length = rectangles_length * 2;
	Slice* x_rectangle_slices = (Slice*)malloc(rectangle_events_length * sizeof(Slice));
	Slice* y_rectangle_slices = (Slice*)malloc(rectangle_events_length * sizeof(Slice));

	for (int i = 0; i < rectangles_length; i++)
	{
		Slice start_event = { rectangles[i].min_x, 0, rectangles[i].min_y, rectangles[i].max_y, i };
		Slice end_event = { rectangles[i].max_x, 1, rectangles[i].min_y, rectangles[i].max_y, i };

		x_rectangle_slices[i * 2] = start_event;
		x_rectangle_slices[i * 2 + 1] = end_event;
	}

	for (int i = 0; i < rectangles_length; i++)
	{
		Slice start_event = { rectangles[i].min_y, 0, rectangles[i].min_x, rectangles[i].max_x, i };
		Slice end_event = { rectangles[i].max_y, 1, rectangles[i].min_x, rectangles[i].max_x, i };

		y_rectangle_slices[i * 2] = start_event;
		y_rectangle_slices[i * 2 + 1] = end_event;
	}

	printf("Calculating perimeter by y...\n");
	int perimeter_y = calculate_perimeter_for_slices(x_rectangle_slices, rectangle_events_length);

	printf("Calculating perimeter by x...\n");
	int perimeter_x = calculate_perimeter_for_slices(y_rectangle_slices, rectangle_events_length);

	printf("Perimeters: x: %i - y: %i\n", perimeter_x, perimeter_y);

	free(x_rectangle_slices);
	free(y_rectangle_slices);

	return perimeter_x + perimeter_y;
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

	int* interval_uses = (int*)malloc((max_max - min_min) * sizeof(int));

	for (int i = 0; i < max_max - min_min; i++)
		interval_uses[i] = 0;

	for (int i = 0; i < slices_length; i++)
	{
		int added_length = 0;

		if (slices[i].type == 0)
			added_length = increase_interval_uses(interval_uses, min_min, max_max, slices[i].min, slices[i].max);
		else
			added_length = decrease_interval_uses(interval_uses, min_min, max_max, slices[i].min, slices[i].max);

		printf("Slice[%d] - rectangle %d - type %d - pos %d - from %d to %d - added %d to perimeter (%d)\n", i, slices[i].rectangle_index + 1, slices[i].type, slices[i].pos, slices[i].min, slices[i].max, added_length, perimeter);

		perimeter += added_length;

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

