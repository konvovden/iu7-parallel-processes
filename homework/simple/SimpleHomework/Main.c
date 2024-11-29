#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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

	int perimeter = calculate_perimeter(rectangles, rectangles_length);

	printf("Total perimeter: %i\n", perimeter);

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

typedef struct {
	int pos; 
	int type; // 0 - begin, 1 - ending
	int min;
	int max;
} Slice;

int compare_rectangle_slices(const void* a, const void* b)
{
	Slice* event_a = (Slice*)a;
	Slice* event_b = (Slice*)b;

	return (event_a->pos - event_b->pos);
}

int calculate_perimeter(Rectangle* rectangles, int rectangles_length)
{
	int rectangle_events_length = rectangles_length * 2;
	Slice* x_rectangle_events = (Slice*)malloc(rectangle_events_length * sizeof(Slice));
	
	int min_y = INT_MAX;
	int max_y = INT_MIN;

	for (int i = 0; i < rectangles_length; i++)
	{
		if (rectangles[i].min_y < min_y)
			min_y = rectangles[i].min_y;

		if (rectangles[i].max_y > max_y)
			max_y = rectangles[i].max_y;

		Slice start_event = { rectangles[i].min_x, 0, &rectangles[i] };
		Slice end_event = { rectangles[i].max_x, 1, &rectangles[i] };

		x_rectangle_events[i * 2] = start_event;
		x_rectangle_events[i * 2 + 1] = end_event;
	}

	qsort(x_rectangle_events, rectangle_events_length, sizeof(Slice), compare_rectangle_events);

	SegmentsTree* tree = build_segments_tree(min_y, max_y);

	int current_x = x_rectangle_events[0].pos;

	for (int i = 0; i < rectangle_events_length; i++)
	{
		Slice current_event = x_rectangle_events[i];

		if (current_event.type == 0) // Begin
			add_interval_to_segments_tree(tree, current_event.rectangle->min_y, current_event.rectangle->max_y);
		else // End
			remove_interval_from_segments_tree(tree, current_event.rectangle->min_y, current_event.rectangle->max_y);
	}

	int perimeter_x = 

	int min_x = INT_MAX;
	int max_x = INT_MIN;

	for (int i = 0; i < rectangles_length; i++)
	{
		if (rectangles[i].min_x < min_x)
			min_x = rectangles[i].min_x;

		if (rectangles[i].max_x > max_x)
			max_x = rectangles[i].max_x;

		Slice start_event = { rectangles[i].min_y, 0, &rectangles[i] };
		Slice end_event = { rectangles[i].max_y, 1, &rectangles[i] };

		x_rectangle_events[i * 2] = start_event;
		x_rectangle_events[i * 2 + 1] = end_event;
	}

	printf("\nPerimeter before: %i\n", perimeter);

	qsort(x_rectangle_events, rectangle_events_length, sizeof(Slice), compare_rectangle_events);

	tree = build_segments_tree(min_x, max_x);

	int current_y = x_rectangle_events[0].pos;

	for (int i = 0; i < rectangle_events_length; i++)
	{
		int added_length = 0;

		Slice current_event = x_rectangle_events[i];

		if (current_event.type == 0) // Begin
			added_length = add_interval_to_segments_tree(tree, current_event.rectangle->min_x, current_event.rectangle->max_x);
		else // End
			added_length = remove_interval_from_segments_tree(tree, current_event.rectangle->min_x, current_event.rectangle->max_x);

		perimeter += added_length;

		current_y = current_event.pos;
	}

	return perimeter;
}

int calculate_perimeter_for_slices(Slice* slices, int slices_length)
{
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

	SegmentsTree* tree = build_segments_tree(min_min, max_max);

	for (int i = 0; i < slices_length; i++)
	{
		if (slices[i].type == 0)
			add_interval_to_segments_tree(tree, slices[i].min, slices[i].max);

	}
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
	result->min = min;
	result->max = max;


	if (min == max)
	{
		result->left = NULL;
		result->right = NULL;
	}
	else
	{
		int middle = min + (max - min) / 2;
		result->left = build_segments_tree_node(min, middle);
		result->right = build_segments_tree_node(middle + 1, max);
	}

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
		int middle = current_node->min + (current_node->max - current_node->min) / 2;

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

		int middle = current_node->min + (current_node->max - current_node->min) / 2;

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
