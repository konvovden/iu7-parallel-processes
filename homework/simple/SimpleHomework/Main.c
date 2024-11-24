#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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
Figure* calculate_perimeter(Rectangle* rectangles, int rectangles_length);

void print_perimeters(Figure* figures, int length);
void print_perimeter(Figure perimeter);

typedef struct {
	struct SegmentsTreeNode* left;
	struct SegmentsTreeNode* right;
	int uses_count;
} SegmentsTreeNode;

typedef struct {
	int min_value;
	int max_value;
	SegmentsTreeNode* root;
} SegmentsTree;

SegmentsTree* build_segments_tree(int min, int max);
SegmentsTreeNode* build_segments_tree_node(int min, int max);
int add_interval_to_segments_tree(SegmentsTree* tree, int min, int max);
int remove_interval_from_segments_tree(SegmentsTree* tree, int min, int max);
int increase_uses_count(SegmentsTree* tree, int node_value);
int decrease_uses_count(SegmentsTree* tree, int node_value);

int main()
{
	Rectangle* rectangles = NULL;
	int rectangles_length;

	generate_example(1, &rectangles, &rectangles_length);

	printf("%i %i %i %i", rectangles[0].min_x, rectangles[0].min_y, rectangles[0].max_x, rectangles[0].max_y);

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

		break;
	}
}

#pragma endregion


#pragma region Calculation

void calculate_perimeters(Rectangle* rectangles, int rectangles_length, Figure** result, int* result_length)
{
	// TODO: Divide to figures

	*result_length = 1;
	*result = (Figure*)malloc(*result_length * sizeof(Figure));

	Figure* ptr = *result;
	ptr[0] = *calculate_perimeter(rectangles, rectangles_length);
}

typedef struct {
	int event_x; 
	int event_type; // 0 - begin, 1 - ending
	Rectangle* rectangle;
} RectangleEvent;

int compare_rectangle_events(const void* a, const void* b)
{
	RectangleEvent* event_a = (RectangleEvent*)a;
	RectangleEvent* event_b = (RectangleEvent*)b;

	return (event_a->event_x - event_b->event_x);
}

Figure* calculate_perimeter(Rectangle* rectangles, int rectangles_length)
{
	int rectangle_events_length = rectangles_length * 2;
	RectangleEvent* rectangle_events = (RectangleEvent*)malloc(rectangle_events_length * sizeof(RectangleEvent));
	
	int min_y = INT_MAX;
	int max_y = INT_MIN;

	for (int i = 0; i < rectangles_length; i++)
	{
		if (rectangles[i].min_y < min_y)
			min_y = rectangles[i].min_y;

		if (rectangles[i].max_y > max_y)
			max_y = rectangles[i].max_y;

		RectangleEvent start_event = { rectangles[i].min_x, 0, &rectangles[i] };
		RectangleEvent end_event = { rectangles[i].max_x, 1, &rectangles[i] };

		rectangle_events[i * 2] = start_event;
		rectangle_events[i * 2 + 1] = end_event;
	}

	qsort(rectangle_events, rectangle_events_length, sizeof(RectangleEvent), compare_rectangle_events);

	SegmentsTree* tree = build_segments_tree(min_y, max_y);

	int current_x = rectangle_events[0].event_x;
	int perimeter = 0;

	for (int i = 0; i < rectangle_events_length; i++)
	{
		int added_length = 0;

		RectangleEvent current_event = rectangle_events[i];

		if (current_event.event_type == 0) // Begin
			added_length = add_interval_to_segments_tree(tree, current_event.rectangle->min_y, current_event.rectangle->max_y);
		else // End
			added_length = remove_interval_from_segments_tree(tree, current_event.rectangle->min_y, current_event.rectangle->max_y);

		perimeter += added_length;
		perimeter += (current_event.event_x - current_x) * 2; // TODO: считать кол-во активных квадратов

		current_x = current_event.event_x;
	}

	Figure* result = (Figure*)malloc(sizeof(Figure));

	result->rectangles = rectangles;
	result->rectangles_count = rectangles_length;
	result->perimeter = perimeter;

	return result;
}

#pragma endregion

#pragma region SegmentsTree

SegmentsTree* build_segments_tree(int min, int max)
{
	SegmentsTree* tree = (SegmentsTree*)malloc(sizeof(SegmentsTree));

	tree->min_value = min;
	tree->max_value = max;
	tree->root = build_segments_tree_node(min, max);

	return tree;
}

SegmentsTreeNode* build_segments_tree_node(int min, int max)
{
	SegmentsTreeNode* result = (SegmentsTreeNode*)malloc(sizeof(SegmentsTreeNode));
	result->uses_count = 0;


	if (min == max)
	{
		result->left = NULL;
		result->right = NULL;
	}
	else if (max > min)
	{
		int middle = min + (max - min) / 2;
		result->left = build_segments_tree_node(min, middle);
		result->right = build_segments_tree_node(middle + 1, max);
	}
	else
		return NULL;

	return result;
}

int add_interval_to_segments_tree(SegmentsTree* tree, int min, int max)
{
	int result = 0;

	for (int i = min; i <= max; i++)
		result += increase_uses_count(tree, i);

	return result;
}

int remove_interval_from_segments_tree(SegmentsTree* tree, int min, int max)
{
	int result = 0;

	for (int i = min; i <= max; i++)
		result += decrease_uses_count(tree, i);

	return result;
}

int increase_uses_count(SegmentsTree* tree, int node_value)
{
	SegmentsTreeNode* current_node = tree->root;
	SegmentsTreeNode* new_node = NULL;
	int i = 0;
	int result = 0;

	while (1)
	{
		i++;
		int middle = tree->min_value + (tree->max_value - tree->min_value) / pow(2, i);

		if (node_value <= middle)
			new_node = current_node->left;
		else
			new_node = current_node->right;

		if (new_node == NULL)
		{
			if (current_node->uses_count == 0)
				result++;

			current_node->uses_count++;
			break;
		}

		current_node = new_node;
	}

	return result;
}

int decrease_uses_count(SegmentsTree* tree, int node_value)
{
	SegmentsTreeNode* current_node = tree->root;
	SegmentsTreeNode* new_node = NULL;
	int i = 0;
	int result = 0;

	while (1)
	{
		i++;

		int middle = tree->min_value + (tree->max_value - tree->min_value) / pow(2, i);

		if (node_value <= middle)
			new_node = current_node->left;
		else
			new_node = current_node->right;

		if (new_node == NULL)
		{
			if (current_node->uses_count == 1)
				result++;

			current_node->uses_count--;
			break;
		}

		current_node = new_node;
	}

	return result;
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