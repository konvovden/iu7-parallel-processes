#include <stdio.h>
#include <stdlib.h>

typedef struct {
	int min_x;
	int min_y;
	int max_x;
	int max_y;
} Rectangle;

typedef struct {
	Rectangle* rectangles;
	int rectangles_count;
	int perimeter;
} Figure;

void generate_example(int example_number, Rectangle** rectangles, int* length);

void calculate_perimeters(Rectangle* rectangles, int rectangles_length, Figure** result, int* result_length);

void print_perimeters(Figure* figures, int length);
void print_perimeter(Figure perimeter);

int add_interval(SegmentsTree* tree, int min, int max);
int remove_interval(SegmentsTree* tree, int min, int max);

int main()
{
	Rectangle* rectangles;
	int rectangles_length;

	generate_example(1, &rectangles, &rectangles_length);

	Figure* perimeters;
	int perimeters_length;

	calculate_perimeters(rectangles, rectangles_length, &perimeters, &perimeters_length);

	print_perimeters(perimeters, perimeters_length);

	return 0;
}

#pragma region Examples

void generate_example(int exampleNumber, Rectangle** rectangles, int* length)
{
	switch (exampleNumber)
	{
	case 1:
		*length = 5;
		Rectangle rect[5] = { {0, 10, 10, 15}, { 5, 13, 17, 18 }, {8, 6, 14, 11}, { 11, 16, 12, 17}, { 13, 4, 15, 20}};
		*rectangles = rect;
		break;
	}
}

#pragma endregion


#pragma region Calculation

void calculate_perimeters(Rectangle* rectangles, int rectangles_length, Figure** result, int* result_length)
{
	// TODO: Divide to figures

	*result_length = 1;
	
	Figure per[1] = { calculate_perimeter(rectangles, rectangles_length) };

	*result = per;
}

typedef struct {
	int start;
	int end;
} Segment;

typedef struct {
	int event_x; 
	int event_type; // 0 - begin, 1 - ending
	Rectangle* rectangle;
} RectangleEvent;

typedef struct {
	Segment* segments;
	int segments_length;
} SegmentsTree;

int compare_rectangle_events(const void* a, const void* b)
{
	RectangleEvent* event_a = (RectangleEvent*)a;
	RectangleEvent* event_b = (RectangleEvent*)b;

	return (event_b->event_x - event_a->event_x);
}

Figure calculate_perimeter(Rectangle* rectangles, int rectangles_length)
{
	int rectangle_events_length = rectangles_length * 2;
	RectangleEvent* rectangle_events = (RectangleEvent*)malloc(rectangle_events_length * sizeof(RectangleEvent));
	

	for (int i = 0; i < rectangles_length; i++)
	{
		RectangleEvent start_event = { rectangles[i].min_x, 0, &rectangles[i] };
		RectangleEvent end_event = { rectangles[i].max_x, 1, &rectangles[i] };

		rectangle_events[i * 2] = start_event;
		rectangle_events[i * 2 + 1] = end_event;
	}

	qsort(rectangle_events, rectangle_events_length, sizeof(RectangleEvent), compare_rectangle_events);

	SegmentsTree tree;

	int current_x = rectangle_events[0].event_x;
	int perimeter = 0;

	for (int i = 0; i < rectangle_events_length; i++)
	{
		RectangleEvent current_event = rectangle_events[i];

		if (current_event.event_type == 0) // Begin
		{
			int added_length = add_interval(&tree, current_event.rectangle->min_y, current_event.rectangle->max_y);
			perimeter = perimeter + added_length;
		}
		else // End
		{
			int removed_length = 
		}
	}

}

#pragma endregion

#pragma region SegmentsTree

void buildSegmentTree(int arr[], int node, int start, int end)
{
	if (start == end) {
		segmentTree[node] = arr[start];
		return;
	}
	int mid = (start + end) / 2;
	buildSegmentTree(arr, 2 * node, start, mid);
	buildSegmentTree(arr, 2 * node + 1, mid + 1, end);
	segmentTree[node]
		= segmentTree[2 * node] + segmentTree[2 * node + 1];
}

// Function to query the segment tree
int query(int node, int start, int end, int l, int r)
{
	if (r < start || end < l)
		return 0;
	if (l <= start && end <= r)
		return segmentTree[node];
	int mid = (start + end) / 2;
	return query(2 * node, start, mid, l, r)
		+ query(2 * node + 1, mid + 1, end, l, r);
}

// Function to update the segment tree
void update(int node, int start, int end, int idx, int val)
{
	if (start == end) {
		segmentTree[node] = val;
		return;
	}
	int mid = (start + end) / 2;
	if (idx <= mid)
		update(2 * node, start, mid, idx, val);
	else
		update(2 * node + 1, mid + 1, end, idx, val);
	segmentTree[node]
		= segmentTree[2 * node] + segmentTree[2 * node + 1];
}

#pragma endregion

#pragma region Output

void print_perimeters(Figure* perimeters, int length)
{
	for (int i = 0; i < length; i++)
	{
		printf("Figure %i: ", i);
		print_perimeter(perimeters[i]);
	}
}


void print_perimeter(Figure perimeter)
{
	printf("Rectangles: %i, total perimeter: %i\n", perimeter.rectangles_count, perimeter.perimeter);
}

#pragma endregion