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
	struct SegmentsTreeNode* left;
	struct SegmentsTreeNode* right;
	int min;
	int max;
	int uses_count;
} SegmentsTreeNode;

typedef struct {
	int min_value;
	int max_value;
	SegmentsTreeNode* root;
} SegmentsTree;

typedef struct {
	int pos;
	int type; // 0 - begin, 1 - ending
	int min;
	int max;
} Slice;

int calculate_perimeter_for_slices(Slice* slices, int slices_length);

int main()
{
	Rectangle* rectangles = NULL;
	int rectangles_length;

	generate_example(1, &rectangles, &rectangles_length);

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
		*length = 7;

		*rectangles = (Rectangle*)malloc(*length * sizeof(Rectangle));
		// 5 + 5 + 3 + 8 + 2 + 2 + 2 + 2 + 5 + 2 + 9 + 2 + 2 + 5 + 4 + 8 + 2 + 3 + 2 + 3
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

		ptr[5].min_x = 18;
		ptr[5].min_y = 5;
		ptr[5].max_x = 20;
		ptr[5].max_y = 10;

		ptr[6].min_x = 20;
		ptr[6].min_y = 2;
		ptr[6].max_x = 22;
		ptr[6].max_y = 7;

		break;
	}
}

#pragma endregion


#pragma region Calculation

int compare_rectangle_slices(const void* a, const void* b)
{
	Slice* event_a = (Slice*)a;
	Slice* event_b = (Slice*)b;

	return (event_a->pos - event_b->pos);
}

int calculate_perimeter(Rectangle* rectangles, int rectangles_length)
{
	int rectangle_events_length = rectangles_length * 2;
	Slice* x_rectangle_slices = (Slice*)malloc(rectangle_events_length * sizeof(Slice));
	Slice* y_rectangle_slices = (Slice*)malloc(rectangle_events_length * sizeof(Slice));

	for (int i = 0; i < rectangles_length; i++)
	{
		Slice start_event = { rectangles[i].min_x, 0, rectangles[i].min_y, rectangles[i].max_y };
		Slice end_event = { rectangles[i].max_x, 1, rectangles[i].min_y, rectangles[i].max_y };

		x_rectangle_slices[i * 2] = start_event;
		x_rectangle_slices[i * 2 + 1] = end_event;
	}

	for (int i = 0; i < rectangles_length; i++)
	{
		Slice start_event = { rectangles[i].min_y, 0, rectangles[i].min_x, rectangles[i].max_x };
		Slice end_event = { rectangles[i].max_y, 1, rectangles[i].min_x, rectangles[i].max_x };

		y_rectangle_slices[i * 2] = start_event;
		y_rectangle_slices[i * 2 + 1] = end_event;
	}

	qsort(x_rectangle_slices, rectangle_events_length, sizeof(Slice), compare_rectangle_slices);

	int perimeter_y = calculate_perimeter_for_slices(x_rectangle_slices, rectangle_events_length);
	int perimeter_x = calculate_perimeter_for_slices(y_rectangle_slices, rectangle_events_length);

	printf("Perimeters: %i %i\n", perimeter_x, perimeter_y);

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

	for (int i = 0; i < slices_length; i++)
	{
		int added_length = 0;

		if (slices[i].type == 0) {
			for (int j = slices[i].min - min_min; j < slices[i].max - min_min; j++) {
				if(interval_uses[j] == 0)
					added_length++;

				interval_uses[j] ++;
			}
		}
		else {
			for (int j = slices[i].min - min_min; j < slices[i].max - min_min; j++) {
				if(interval_uses[j] == 1)
					added_length++;

				interval_uses[j] --;
			}
		}

		perimeter += added_length;

	}

	free(interval_uses);

	return perimeter;
}

#pragma endregion

