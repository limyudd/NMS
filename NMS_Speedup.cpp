#include <iostream>
#include <algorithm>
#include <time.h>

using namespace std;

const float threshold = 0.24;


typedef struct Box
{
	float x, y, w, h;
	float l, r, u, d;
}Box;

typedef struct Detection
{
	Box box;
	float s;
	float probs;
	int class_id;
	int oriIndex;
	int nextclassIndex;
}Detection;

struct Comparator {
	bool operator()(const Detection& det1, const Detection& det2) const {
		if (det1.class_id < det2.class_id) return true;
		if (det1.class_id == det2.class_id) {
			return (det1.probs > det2.probs);
		}
		return false;
	}
};

struct ComparatorIndex {
	bool operator()(const Detection& det1, const Detection& det2) const {
		return det1.oriIndex < det2.oriIndex;
	}
};


void do_nms_sort(Detection *dets, int total, int classes, float nms_thresh)
{
	if (total <= 0) return;

	//sort(dets, dets + total, nms_compare);
	sort(dets, dets + total, Comparator());
	float w1, h1, left, right, up, down;
	float h, in, s1, s2, w, s, unionArea;
	float iou;
	float iouArea;
	float temp;
	float *record = NULL;
	if (record == NULL) record = (float*)calloc(total, sizeof(float));
	//int flag = total -1;
	int lastclass = dets[total - 1].class_id;
	dets[total - 1].nextclassIndex = total - 1;
	
	for (int i = total - 2; i >= 0; i--)
	{
		if (lastclass == dets[i].class_id)
		{
			dets[i].nextclassIndex = dets[i + 1].nextclassIndex;
		}
		else
		{
			dets[i].nextclassIndex = i;
			//flag = i;
			lastclass = dets[i].class_id;
		}

	}

#pragma omp parallel
#pragma omp for

	for (int i = 0; i < total; i++)
	{
		dets[i].s = nms_thresh * dets[i].box.h * dets[i].box.w;
		//dets[i].probs = dets[i].probs < threshold ? 0 : dets[i].probs;
		w1 = dets[i].box.w * 0.5f;
		h1 = dets[i].box.h * 0.5f;
		dets[i].box.l = dets[i].box.x - w1;
		dets[i].box.r = dets[i].box.x + w1;
		dets[i].box.u = dets[i].box.y - h1;
		dets[i].box.d = dets[i].box.y + h1;
		temp = 1.0f + nms_thresh;
		dets[i].box.l = temp * dets[i].box.l;
		dets[i].box.r = temp * dets[i].box.r;
		/*dets[i].box.u = nms_thresh * dets[i].box.u;
		dets[i].box.d = nms_thresh * dets[i].box.d;*/

	}

#pragma omp parallel
#pragma omp for

	for (int i = 0; i < total; i++)
	{
		if (dets[i].probs == 0) continue;

		if (dets[i].box.w == 0 || dets[i].box.h == 0) continue;
		s1 = dets[i].s;
		int nextId = dets[i].nextclassIndex;
#pragma omp parallel
#pragma omp for
		int j;
		for (j = i + 1; j <= nextId; j++) {
			//if (dets[j].probs == 0) continue;
			//if (dets[j].probs < threshold) break;
			//if (dets[i].class_id != dets[j].class_id) break;
			//if (dets[j].box.w == 0 || dets[j].box.h == 0) continue;
			
			left = dets[i].box.l > dets[j].box.l ? dets[i].box.l : dets[j].box.l;
			right = dets[i].box.r < dets[j].box.r ? dets[i].box.r : dets[j].box.r;
			w = right - left;
			
			w = 0 > w ? 0 : w;

			up = dets[i].box.u > dets[j].box.u ? dets[i].box.u : dets[j].box.u;
			down = dets[i].box.d < dets[j].box.d ? dets[i].box.d : dets[j].box.d;
			h = down - up;
	
			h = 0 > h ? 0 : h;

			in = w * h;

			s = s1 + dets[j].s;
			
			dets[j].probs = in > s ? 0 : dets[j].probs;

			/*left = dets[i].box.l > dets[j + 1].box.l ? dets[i].box.l : dets[j + 1].box.l;
			right = dets[i].box.r < dets[j + 1].box.r ? dets[i].box.r : dets[j + 1].box.r;
			w = right - left;
			if (w <= 0)
			{
				continue;
			}
			up = dets[i].box.u > dets[j + 1].box.u ? dets[i].box.u : dets[j + 1].box.u;
			down = dets[i].box.d < dets[j + 1].box.d ? dets[i].box.d : dets[j + 1].box.d;
			h = down - up;

			if (h <= 0)
			{
				continue;
			}

			in = w * h;

			s = s1 + dets[j + 1].s;

			dets[j + 1].probs = in > s ? 0 : dets[j + 1].probs;*/
			
		}
		//for (; j <= nextId; j++) {

		//	left = dets[i].box.l > dets[j].box.l ? dets[i].box.l : dets[j].box.l;
		//	right = dets[i].box.r < dets[j].box.r ? dets[i].box.r : dets[j].box.r;
		//	w = right - left;

		//	if (w <= 0)
		//	{
		//		continue;
		//	}

		//	up = dets[i].box.u > dets[j].box.u ? dets[i].box.u : dets[j].box.u;
		//	down = dets[i].box.d < dets[j].box.d ? dets[i].box.d : dets[j].box.d;
		//	h = down - up;

		//	if (h <= 0)
		//	{
		//		continue;
		//	}

		//	in = w * h;// *(1.0f + nms_thresh);

		//	s = s1 + dets[j].s;
		//	//unionArea = s - in;
		//	//iou = in / unionArea;
		//	//iouArea = s - in;

		//	dets[j].probs = in > s ? 0 : dets[j].probs;
		//	//record[j] = in - s;
		//}
	}
	
	sort(dets, dets + total, ComparatorIndex());
}



int main()
{
	int total = 163840;
	int classes = 20;

	Box *boxes = NULL;
	if (boxes == NULL)boxes = (Box*)calloc(total, sizeof(Box));


	Detection *dets = NULL;
	if (dets == NULL) dets = (Detection *)calloc(total, sizeof(Detection));
	int t = 1;
	while(t--)
	{
	
	float score;
	float x, y, w, h;
	int classId;
	int index = 0;
	int detId = 0;
	freopen("C:\\Users\\PQMSI\\Desktop\\NMS\\test\\large.in", "r", stdin);
	freopen("C:\\Users\\PQMSI\\Desktop\\NMS\\test\\largeout_sort.txt", "w", stdout);

	while (scanf("%d%f%f%f%f%f", &classId, &score, &x, &y, &w, &h) != EOF) {
		boxes[index].x = x;
		boxes[index].y = y;
		boxes[index].w = w;
		boxes[index].h = h;
		if (score >= 0.24f)
		{
			dets[detId].box = boxes[index];
			dets[detId].probs = score;

			dets[detId].class_id = classId;
			dets[detId].oriIndex = index;
			detId++;
		}
		index++;
	}
	//total = index;
	total = detId;
	float nms_thresh = 0.4;
	clock_t start = clock();
	do_nms_sort(dets, total, classes, nms_thresh);

	
	for (int i = 0; i < total; i++) {
		float prob = dets[i].probs;
		if (prob < .24) continue;
		printf("%d %f %f %f %f %f\n", dets[i].class_id, prob, dets[i].box.x, dets[i].box.y, dets[i].box.w, dets[i].box.h);
	}
	printf("%d******\n", clock() - start);
	}
	if (boxes) {
		free(boxes);
		boxes = NULL;
	}


	if (dets)
	{
		free(dets);
		dets = NULL;
	}
	return 0;
}