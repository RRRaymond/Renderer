#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include <windows.h>
#include <tchar.h>

typedef unsigned int IUINT32;

typedef struct { float m[4][4]; } matrix_t;
typedef struct { float x, y, z, w; } vector_t;
typedef vector_t point_t;
typedef struct { float x, y, z; } direction_t;

// ��bmpͼƬ
void ImageRead(int *width, int *height, int *r, int *g, int *b)
{
	char name[] = "11.bmp";
	//read image
	FILE *fp;
	if ((fp = fopen(name, "rb")) == NULL) {
		printf("cannot open bmp.name\n");
		return;
	}

	WORD bfType, bfReserved1, bfReserved2;
	DWORD bfSize, bfOffBits;
	BITMAPINFOHEADER bih;

	fread(&bfType, sizeof(WORD), 1, fp);
	if (bfType != 0x4D42) {//��ֵ������0x4D42��Ҳ�����ַ�'BM'
		printf("the input map is not bmp type");
		//return;
	}
	fread(&bfSize, sizeof(DWORD), 1, fp);
	fread(&bfReserved1, sizeof(WORD), 1, fp);
	fread(&bfReserved2, sizeof(WORD), 1, fp);
	fread(&bfOffBits, sizeof(DWORD), 1, fp);
	fread(&bih, sizeof(BITMAPINFOHEADER), 1, fp);
	*width = bih.biWidth;
	*height = bih.biHeight;
	if (*width % 4 != 0)
		*width = *width + (4 - *width % 4);
	DWORD size = *width * (bih.biBitCount / 8)*(*height);
	//unsigned char* pData = new unsigned char[size];
	unsigned char* pData = (unsigned char*)malloc(size * sizeof(char));

	fread(pData, size, 1, fp);

	int bmWidthBytes = *width * bih.biBitCount / 8;
	int bmBitsPixel = bih.biBitCount;
	int nBit = bmBitsPixel / 8;

	int z;

	for (int y = 0; y<(*height); y++)
	{
		for (int x = 0; x<(*width); x++)
		{
			z = (*height) - y - 1;
			*(r + (y* (*width) + x)) = pData[x*nBit + 2 + z * bmWidthBytes];
			*(g + (y* (*width) + x)) = pData[x*nBit + 1 + z * bmWidthBytes];
			*(b + (y* (*width) + x)) = pData[x*nBit + z * bmWidthBytes];
		}
	}
}

int CMID(int x, int min, int max) { return (x < min) ? min : ((x > max) ? max : x); }

// �����ֵ��t Ϊ [0, 1] ֮�����ֵ
float interp(float x1, float x2, float t) { return x1 + (x2 - x1) * t; }

// | v |
float vector_length(const vector_t *v) {
	float sq = v->x * v->x + v->y * v->y + v->z * v->z;
	return (float)sqrt(sq);
}

// z = x + y
void vector_add(vector_t *z, const vector_t *x, const vector_t *y) {
	z->x = x->x + y->x;
	z->y = x->y + y->y;
	z->z = x->z + y->z;
	z->w = 1.0;
}

// z = x - y
void vector_sub(vector_t *z, const vector_t *x, const vector_t *y) {
	z->x = x->x - y->x;
	z->y = x->y - y->y;
	z->z = x->z - y->z;
	z->w = 1.0;
}

// ʸ�����
float vector_dotproduct(const vector_t *x, const vector_t *y) {
	return x->x * y->x + x->y * y->y + x->z * y->z;
}

// ʸ�����
void vector_crossproduct(vector_t *z, const vector_t *x, const vector_t *y) {
	float m1, m2, m3;
	m1 = x->y * y->z - x->z * y->y;
	m2 = x->z * y->x - x->x * y->z;
	m3 = x->x * y->y - x->y * y->x;
	z->x = m1;
	z->y = m2;
	z->z = m3;
	z->w = 1.0f;
}

// 3άʸ�����
void vector3_crossproduct(direction_t *z, const direction_t *x, const direction_t *y) {
	float m1, m2, m3;
	m1 = x->y * y->z - x->z * y->y;
	m2 = x->z * y->x - x->x * y->z;
	m3 = x->x * y->y - x->y * y->x;
	z->x = m1;
	z->y = m2;
	z->z = m3;
}

// ʸ����ֵ��tȡֵ [0, 1]
void vector_interp(vector_t *z, const vector_t *x1, const vector_t *x2, float t) {
	z->x = interp(x1->x, x2->x, t);
	z->y = interp(x1->y, x2->y, t);
	z->z = interp(x1->z, x2->z, t);
	z->w = 1.0f;
}

// ʸ����һ��
void vector_normalize(vector_t *v) {
	float length = vector_length(v);
	if (length != 0.0f) {
		float inv = 1.0f / length;
		v->x *= inv;
		v->y *= inv;
		v->z *= inv;
	}
}

// c = a + b
void matrix_add(matrix_t *c, const matrix_t *a, const matrix_t *b) {
	int i, j;
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++)
			c->m[i][j] = a->m[i][j] + b->m[i][j];
	}
}

// c = a - b
void matrix_sub(matrix_t *c, const matrix_t *a, const matrix_t *b) {
	int i, j;
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++)
			c->m[i][j] = a->m[i][j] - b->m[i][j];
	}
}

// c = a * b
void matrix_mul(matrix_t *c, const matrix_t *a, const matrix_t *b) {
	matrix_t z;
	int i, j;
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			z.m[j][i] = (a->m[j][0] * b->m[0][i]) +
				(a->m[j][1] * b->m[1][i]) +
				(a->m[j][2] * b->m[2][i]) +
				(a->m[j][3] * b->m[3][i]);
		}
	}
	c[0] = z;
}

// c = a * f
void matrix_scale(matrix_t *c, const matrix_t *a, float f) {
	int i, j;
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++)
			c->m[i][j] = a->m[i][j] * f;
	}
}

// y = x * m
void matrix_apply(vector_t *y, const vector_t *x, const matrix_t *m) {
	float X = x->x, Y = x->y, Z = x->z, W = x->w;
	y->x = X * m->m[0][0] + Y * m->m[1][0] + Z * m->m[2][0] + W * m->m[3][0];
	y->y = X * m->m[0][1] + Y * m->m[1][1] + Z * m->m[2][1] + W * m->m[3][1];
	y->z = X * m->m[0][2] + Y * m->m[1][2] + Z * m->m[2][2] + W * m->m[3][2];
	y->w = X * m->m[0][3] + Y * m->m[1][3] + Z * m->m[2][3] + W * m->m[3][3];
}

void matrix_set_identity(matrix_t *m) {
	m->m[0][0] = m->m[1][1] = m->m[2][2] = m->m[3][3] = 1.0f;
	m->m[0][1] = m->m[0][2] = m->m[0][3] = 0.0f;
	m->m[1][0] = m->m[1][2] = m->m[1][3] = 0.0f;
	m->m[2][0] = m->m[2][1] = m->m[2][3] = 0.0f;
	m->m[3][0] = m->m[3][1] = m->m[3][2] = 0.0f;
}

void matrix_set_zero(matrix_t *m) {
	m->m[0][0] = m->m[0][1] = m->m[0][2] = m->m[0][3] = 0.0f;
	m->m[1][0] = m->m[1][1] = m->m[1][2] = m->m[1][3] = 0.0f;
	m->m[2][0] = m->m[2][1] = m->m[2][2] = m->m[2][3] = 0.0f;
	m->m[3][0] = m->m[3][1] = m->m[3][2] = m->m[3][3] = 0.0f;
}

// ƽ�Ʊ任
void matrix_set_translate(matrix_t *m, float x, float y, float z) {
	matrix_set_identity(m);
	m->m[3][0] = x;
	m->m[3][1] = y;
	m->m[3][2] = z;
}

// ���ű任
void matrix_set_scale(matrix_t *m, float x, float y, float z) {
	matrix_set_identity(m);
	m->m[0][0] = x;
	m->m[1][1] = y;
	m->m[2][2] = z;
}

// ��ת����
void matrix_set_rotate(matrix_t *m, float x, float y, float z, float theta) {
	float qsin = (float)sin(theta * 0.5f);
	float qcos = (float)cos(theta * 0.5f);
	vector_t vec = { x, y, z, 1.0f };
	float w = qcos;
	vector_normalize(&vec);
	x = vec.x * qsin;
	y = vec.y * qsin;
	z = vec.z * qsin;
	m->m[0][0] = 1 - 2 * y * y - 2 * z * z;
	m->m[1][0] = 2 * x * y - 2 * w * z;
	m->m[2][0] = 2 * x * z + 2 * w * y;
	m->m[0][1] = 2 * x * y + 2 * w * z;
	m->m[1][1] = 1 - 2 * x * x - 2 * z * z;
	m->m[2][1] = 2 * y * z - 2 * w * x;
	m->m[0][2] = 2 * x * z - 2 * w * y;
	m->m[1][2] = 2 * y * z + 2 * w * x;
	m->m[2][2] = 1 - 2 * x * x - 2 * y * y;
	m->m[0][3] = m->m[1][3] = m->m[2][3] = 0.0f;
	m->m[3][0] = m->m[3][1] = m->m[3][2] = 0.0f;
	m->m[3][3] = 1.0f;
}

// ���������
void matrix_set_lookat(matrix_t *m, const vector_t *eye, const vector_t *at, const vector_t *up) {
	vector_t xaxis, yaxis, zaxis;

	vector_sub(&zaxis, at, eye);
	vector_normalize(&zaxis);
	vector_crossproduct(&xaxis, up, &zaxis);
	vector_normalize(&xaxis);
	vector_crossproduct(&yaxis, &zaxis, &xaxis);

	m->m[0][0] = xaxis.x;
	m->m[1][0] = xaxis.y;
	m->m[2][0] = xaxis.z;
	m->m[3][0] = -vector_dotproduct(&xaxis, eye);

	m->m[0][1] = yaxis.x;
	m->m[1][1] = yaxis.y;
	m->m[2][1] = yaxis.z;
	m->m[3][1] = -vector_dotproduct(&yaxis, eye);

	m->m[0][2] = zaxis.x;
	m->m[1][2] = zaxis.y;
	m->m[2][2] = zaxis.z;
	m->m[3][2] = -vector_dotproduct(&zaxis, eye);

	m->m[0][3] = m->m[1][3] = m->m[2][3] = 0.0f;
	m->m[3][3] = 1.0f;
}

// D3DXMatrixPerspectiveFovLH
void matrix_set_perspective(matrix_t *m, float fovy, float aspect, float zn, float zf) {
	float fax = 1.0f / (float)tan(fovy * 0.5f);
	matrix_set_zero(m);
	m->m[0][0] = (float)(fax / aspect);
	m->m[1][1] = (float)(fax);
	m->m[2][2] = zf / (zf - zn);
	m->m[3][2] = -zn * zf / (zf - zn);
	m->m[2][3] = 1;
}


//=====================================================================
// ����任
//=====================================================================
typedef struct {
	matrix_t world;         // ��������任
	matrix_t view;          // ��Ӱ������任
	matrix_t projection;    // ͶӰ�任
	matrix_t transform;     // transform = world * view * projection
	float w, h;             // ��Ļ��С
}	transform_t;


// ������£����� transform = world * view * projection
void transform_update(transform_t *ts) {
	matrix_t m;
	matrix_mul(&m, &ts->world, &ts->view);
	matrix_mul(&ts->transform, &m, &ts->projection);
}

// ��ʼ����������Ļ����
void transform_init(transform_t *ts, int width, int height) {
	float aspect = (float)width / ((float)height);
	matrix_set_identity(&ts->world);
	matrix_set_identity(&ts->view);
	matrix_set_perspective(&ts->projection, 3.1415926f * 0.5f, aspect, 1.0f, 500.0f);
	ts->w = (float)width;
	ts->h = (float)height;
	transform_update(ts);
}

// ��ʸ�� x ���� project 
void transform_apply(const transform_t *ts, vector_t *y, const vector_t *x) {
	matrix_apply(y, x, &ts->transform);
}

// ����������ͬ cvv �ı߽�������׶�ü�
int transform_check_cvv(const vector_t *v) {
	float w = v->w;
	int check = 0;
	if (v->z < 0.0f) check |= 1;
	if (v->z >  w) check |= 2;
	if (v->x < -w) check |= 4;
	if (v->x >  w) check |= 8;
	if (v->y < -w) check |= 16;
	if (v->y >  w) check |= 32;
	return check;
}

// ��һ�����õ���Ļ����
void transform_homogenize(const transform_t *ts, vector_t *y, const vector_t *x) {
	float rhw = 1.0f / x->w;
	y->x = (x->x * rhw + 1.0f) * ts->w * 0.5f;
	y->y = (1.0f - x->y * rhw) * ts->h * 0.5f;
	y->z = x->z * rhw;
	y->w = 1.0f;
}


//=====================================================================
// ���μ��㣺���㡢ɨ���ߡ���Ե�����Ρ���������
//=====================================================================
typedef struct { float r, g, b; } color_t;
typedef struct { float u, v; } texcoord_t;
typedef struct { point_t pos; texcoord_t tc; color_t color; float rhw; } vertex_t;

typedef struct { vertex_t v, v1, v2; } edge_t;
typedef struct { float top, bottom; edge_t left, right; } trapezoid_t;
typedef struct { vertex_t v, step; int x, y, w; } scanline_t;


void vertex_rhw_init(vertex_t *v) {
	float rhw = 1.0f / v->pos.w;
	v->rhw = rhw;
	v->tc.u *= rhw;
	v->tc.v *= rhw;
	v->color.r *= rhw;
	v->color.g *= rhw;
	v->color.b *= rhw;
}

void vertex_interp(vertex_t *y, const vertex_t *x1, const vertex_t *x2, float t) {
	vector_interp(&y->pos, &x1->pos, &x2->pos, t);
	y->tc.u = interp(x1->tc.u, x2->tc.u, t);
	y->tc.v = interp(x1->tc.v, x2->tc.v, t);
	y->color.r = interp(x1->color.r, x2->color.r, t);
	y->color.g = interp(x1->color.g, x2->color.g, t);
	y->color.b = interp(x1->color.b, x2->color.b, t);
	y->rhw = interp(x1->rhw, x2->rhw, t);
}

void vertex_division(vertex_t *y, const vertex_t *x1, const vertex_t *x2, float w) {
	float inv = 1.0f / w;
	y->pos.x = (x2->pos.x - x1->pos.x) * inv;
	y->pos.y = (x2->pos.y - x1->pos.y) * inv;
	y->pos.z = (x2->pos.z - x1->pos.z) * inv;
	y->pos.w = (x2->pos.w - x1->pos.w) * inv;
	y->tc.u = (x2->tc.u - x1->tc.u) * inv;
	y->tc.v = (x2->tc.v - x1->tc.v) * inv;
	y->color.r = (x2->color.r - x1->color.r) * inv;
	y->color.g = (x2->color.g - x1->color.g) * inv;
	y->color.b = (x2->color.b - x1->color.b) * inv;
	y->rhw = (x2->rhw - x1->rhw) * inv;
}

void vertex_add(vertex_t *y, const vertex_t *x) {
	y->pos.x += x->pos.x;
	y->pos.y += x->pos.y;
	y->pos.z += x->pos.z;
	y->pos.w += x->pos.w;
	y->rhw += x->rhw;
	y->tc.u += x->tc.u;
	y->tc.v += x->tc.v;
	y->color.r += x->color.r;
	y->color.g += x->color.g;
	y->color.b += x->color.b;
}

// �������������� 0-2 �������Σ����ҷ��غϷ������ε�����
int trapezoid_init_triangle(trapezoid_t *trap, const vertex_t *p1,
	const vertex_t *p2, const vertex_t *p3) {
	const vertex_t *p;
	float k, x;

	if (p1->pos.y > p2->pos.y) p = p1, p1 = p2, p2 = p;
	if (p1->pos.y > p3->pos.y) p = p1, p1 = p3, p3 = p;
	if (p2->pos.y > p3->pos.y) p = p2, p2 = p3, p3 = p;
	if (p1->pos.y == p2->pos.y && p1->pos.y == p3->pos.y) return 0;
	if (p1->pos.x == p2->pos.x && p1->pos.x == p3->pos.x) return 0;

	if (p1->pos.y == p2->pos.y) {	// triangle down
		if (p1->pos.x > p2->pos.x) p = p1, p1 = p2, p2 = p;
		trap[0].top = p1->pos.y;
		trap[0].bottom = p3->pos.y;
		trap[0].left.v1 = *p1;
		trap[0].left.v2 = *p3;
		trap[0].right.v1 = *p2;
		trap[0].right.v2 = *p3;
		return (trap[0].top < trap[0].bottom) ? 1 : 0;
	}

	if (p2->pos.y == p3->pos.y) {	// triangle up
		if (p2->pos.x > p3->pos.x) p = p2, p2 = p3, p3 = p;
		trap[0].top = p1->pos.y;
		trap[0].bottom = p3->pos.y;
		trap[0].left.v1 = *p1;
		trap[0].left.v2 = *p2;
		trap[0].right.v1 = *p1;
		trap[0].right.v2 = *p3;
		return (trap[0].top < trap[0].bottom) ? 1 : 0;
	}

	trap[0].top = p1->pos.y;
	trap[0].bottom = p2->pos.y;
	trap[1].top = p2->pos.y;
	trap[1].bottom = p3->pos.y;

	k = (p3->pos.y - p1->pos.y) / (p2->pos.y - p1->pos.y);
	x = p1->pos.x + (p2->pos.x - p1->pos.x) * k;

	if (x <= p3->pos.x) {		// triangle left
		trap[0].left.v1 = *p1;
		trap[0].left.v2 = *p2;
		trap[0].right.v1 = *p1;
		trap[0].right.v2 = *p3;
		trap[1].left.v1 = *p2;
		trap[1].left.v2 = *p3;
		trap[1].right.v1 = *p1;
		trap[1].right.v2 = *p3;
	}
	else {					// triangle right
		trap[0].left.v1 = *p1;
		trap[0].left.v2 = *p3;
		trap[0].right.v1 = *p1;
		trap[0].right.v2 = *p2;
		trap[1].left.v1 = *p1;
		trap[1].left.v2 = *p3;
		trap[1].right.v1 = *p2;
		trap[1].right.v2 = *p3;
	}

	return 2;
}

// ���� Y ��������������������������� Y �Ķ���
void trapezoid_edge_interp(trapezoid_t *trap, float y) {
	float s1 = trap->left.v2.pos.y - trap->left.v1.pos.y;
	float s2 = trap->right.v2.pos.y - trap->right.v1.pos.y;
	float t1 = (y - trap->left.v1.pos.y) / s1;
	float t2 = (y - trap->right.v1.pos.y) / s2;
	vertex_interp(&trap->left.v, &trap->left.v1, &trap->left.v2, t1);
	vertex_interp(&trap->right.v, &trap->right.v1, &trap->right.v2, t2);
}

// �����������ߵĶ˵㣬��ʼ�������ɨ���ߵ����Ͳ���
void trapezoid_init_scan_line(const trapezoid_t *trap, scanline_t *scanline, int y) {
	float width = trap->right.v.pos.x - trap->left.v.pos.x;
	scanline->x = (int)(trap->left.v.pos.x + 0.5f);
	scanline->w = (int)(trap->right.v.pos.x + 0.5f) - scanline->x;
	scanline->y = y;
	scanline->v = trap->left.v;
	if (trap->left.v.pos.x >= trap->right.v.pos.x) scanline->w = 0;
	vertex_division(&scanline->step, &trap->left.v, &trap->right.v, width);
}


//=====================================================================
// ��Ⱦ�豸
//=====================================================================
typedef struct {
	transform_t transform;      // ����任��
	int width;                  // ���ڿ��
	int height;                 // ���ڸ߶�
	IUINT32 **framebuffer;      // ���ػ��棺framebuffer[y] ����� y��
	float **zbuffer;            // ��Ȼ��棺zbuffer[y] Ϊ�� y��ָ��
	IUINT32 **texture;          // ����ͬ����ÿ������
	int tex_width;              // ������
	int tex_height;             // ����߶�
	float max_u;                // ��������ȣ�tex_width - 1
	float max_v;                // �������߶ȣ�tex_height - 1
	int render_state;           // ��Ⱦ״̬
	IUINT32 background;         // ������ɫ
	IUINT32 foreground;         // �߿���ɫ
}	device_t;

#define RENDER_STATE_WIREFRAME      1		// ��Ⱦ�߿�
#define RENDER_STATE_TEXTURE        2		// ��Ⱦ����
#define RENDER_STATE_COLOR          4		// ��Ⱦ��ɫ
#define RENDER_STATE_IMG            8       // ��ȾͼƬ

// �豸��ʼ����fbΪ�ⲿ֡���棬�� NULL �������ⲿ֡���棨ÿ�� 4�ֽڶ��룩
void device_init(device_t *device, int width, int height, void *fb) {
	int need = sizeof(void*) * (height * 2 + 1024) + width * height * 8;
	char *ptr = (char*)malloc(need + 64);
	char *framebuf, *zbuf;
	int j;
	assert(ptr);
	device->framebuffer = (IUINT32**)ptr;
	device->zbuffer = (float**)(ptr + sizeof(void*) * height);
	ptr += sizeof(void*) * height * 2;
	device->texture = (IUINT32**)ptr;
	ptr += sizeof(void*) * 1024;
	framebuf = (char*)ptr;
	zbuf = (char*)ptr + width * height * 4;
	ptr += width * height * 8;
	if (fb != NULL) framebuf = (char*)fb;
	for (j = 0; j < height; j++) {
		device->framebuffer[j] = (IUINT32*)(framebuf + width * 4 * j);
		device->zbuffer[j] = (float*)(zbuf + width * 4 * j);
	}
	device->texture[0] = (IUINT32*)ptr;
	device->texture[1] = (IUINT32*)(ptr + 16);
	memset(device->texture[0], 0, 64);
	device->tex_width = 2;
	device->tex_height = 2;
	device->max_u = 1.0f;
	device->max_v = 1.0f;
	device->width = width;
	device->height = height;
	device->background = 0xc0c0c0;
	device->foreground = 0;
	transform_init(&device->transform, width, height);
	device->render_state = RENDER_STATE_WIREFRAME;
}

// ɾ���豸
void device_destroy(device_t *device) {
	if (device->framebuffer)
		free(device->framebuffer);
	device->framebuffer = NULL;
	device->zbuffer = NULL;
	device->texture = NULL;
}

// ���õ�ǰ����
void device_set_texture(device_t *device, void *bits, long pitch, int w, int h) {
	char *ptr = (char*)bits;
	int j;
	assert(w <= 1024 && h <= 1024);
	for (j = 0; j < h; ptr += pitch, j++) 	// ���¼���ÿ�������ָ��
		device->texture[j] = (IUINT32*)ptr;
	device->tex_width = w;
	device->tex_height = h;
	device->max_u = (float)(w - 1);
	device->max_v = (float)(h - 1);
}

// ��� framebuffer �� zbuffer
void device_clear(device_t *device, int mode) {
	int y, x, height = device->height;
	for (y = 0; y < device->height; y++) {
		IUINT32 *dst = device->framebuffer[y];
		IUINT32 cc = (height - 1 - y) * 230 / (height - 1);
		cc = (cc << 16) | (cc << 8) | cc;
		if (mode == 0) cc = device->background;
		for (x = device->width; x > 0; dst++, x--) dst[0] = cc;
	}
	for (y = 0; y < device->height; y++) {
		float *dst = device->zbuffer[y];
		for (x = device->width; x > 0; dst++, x--) dst[0] = 0.0f;
	}
}

// ����
void device_pixel(device_t *device, int x, int y, IUINT32 color) {
	if (((IUINT32)x) < (IUINT32)device->width && ((IUINT32)y) < (IUINT32)device->height) {
		device->framebuffer[y][x] = color;
	}
}

// �����߶�
void device_draw_line(device_t *device, int x1, int y1, int x2, int y2, IUINT32 c) {
	int x, y, rem = 0;
	if (x1 == x2 && y1 == y2) {
		device_pixel(device, x1, y1, c);
	}
	else if (x1 == x2) {
		int inc = (y1 <= y2) ? 1 : -1;
		for (y = y1; y != y2; y += inc) device_pixel(device, x1, y, c);
		device_pixel(device, x2, y2, c);
	}
	else if (y1 == y2) {
		int inc = (x1 <= x2) ? 1 : -1;
		for (x = x1; x != x2; x += inc) device_pixel(device, x, y1, c);
		device_pixel(device, x2, y2, c);
	}
	else {
		int dx = (x1 < x2) ? x2 - x1 : x1 - x2;
		int dy = (y1 < y2) ? y2 - y1 : y1 - y2;
		if (dx >= dy) {
			if (x2 < x1) x = x1, y = y1, x1 = x2, y1 = y2, x2 = x, y2 = y;
			for (x = x1, y = y1; x <= x2; x++) {
				device_pixel(device, x, y, c);
				rem += dy;
				if (rem >= dx) {
					rem -= dx;
					y += (y2 >= y1) ? 1 : -1;
					device_pixel(device, x, y, c);
				}
			}
			device_pixel(device, x2, y2, c);
		}
		else {
			if (y2 < y1) x = x1, y = y1, x1 = x2, y1 = y2, x2 = x, y2 = y;
			for (x = x1, y = y1; y <= y2; y++) {
				device_pixel(device, x, y, c);
				rem += dx;
				if (rem >= dy) {
					rem -= dy;
					x += (x2 >= x1) ? 1 : -1;
					device_pixel(device, x, y, c);
				}
			}
			device_pixel(device, x2, y2, c);
		}
	}
}

void Hex2RGB(const IUINT32* texture, int* R, int* G, int* B) {
	*B = *texture % 256;
	*G = (*texture % 65536 - *B) / 256;
	*R = (*texture - *G * 256 - *B) / 65536;
}

void RGB2Hex(IUINT32* texture, int R, int G, int B) {
	*texture = R * 65536 + G * 256 + B;
}

float bilinearInterp(int ff, int fc, int cf, int cc, int du, int dv) {
	float temp1 = interp(ff, fc, du);
	float temp2 = interp(cf, cc, du);
	return interp(temp1, temp2, dv);
}

// ���������ȡ����
IUINT32 device_texture_read(const device_t *device, float u, float v) {
	u = u * device->max_u;
	v = v * device->max_v;

	int u_floor = floor(u);
	int v_floor = floor(v);
	int u_ceil = ceil(u);
	int v_ceil = ceil(v);
	float du = u - u_floor;
	float dv = v - v_floor;

	IUINT32 **texture = device->texture;
	IUINT32 texture_ff = texture[v_floor][u_floor];
	IUINT32 texture_fc = texture[v_floor][u_ceil];
	IUINT32 texture_cf = texture[v_ceil][u_floor];
	IUINT32 texture_cc = texture[v_ceil][u_ceil];

	//��4�����ڵ�����ת��Ϊ4��RGBֵ
	int r_ff, g_ff, b_ff, r_fc, g_fc, b_fc, r_cf, g_cf, b_cf, r_cc, g_cc, b_cc;
	Hex2RGB(&texture_ff, &r_ff, &g_ff, &b_ff);
	Hex2RGB(&texture_fc, &r_fc, &g_fc, &b_fc);
	Hex2RGB(&texture_cf, &r_cf, &g_cf, &b_cf);
	Hex2RGB(&texture_cc, &r_cc, &g_cc, &b_cc);

	//����˫���Բ�ֵ����µ�RGBֵ
	float R, G, B;
	R = bilinearInterp(r_ff, r_fc, r_cf, r_cc, du, dv);
	G = bilinearInterp(g_ff, g_fc, g_cf, g_cc, du, dv);
	B = bilinearInterp(b_ff, b_fc, b_cf, b_cc, du, dv);

	//�����ת��Ϊ16���Ʋ�����
	IUINT32 m_text;
	RGB2Hex(&m_text, R, G, B);
	return m_text;
}

//=====================================================================
// ��Ⱦʵ��
//=====================================================================

// ����ɨ����
void device_draw_scanline(device_t *device, scanline_t *scanline) {
	IUINT32 *framebuffer = device->framebuffer[scanline->y];
	float *zbuffer = device->zbuffer[scanline->y];
	int x = scanline->x;
	int w = scanline->w;
	int width = device->width;
	int render_state = device->render_state;
	for (; w > 0; x++, w--) {
		if (x >= 0 && x < width) {
			float rhw = scanline->v.rhw;
			if (rhw >= zbuffer[x]) {
				float w = 1.0f / rhw;
				zbuffer[x] = rhw;
				if (render_state & RENDER_STATE_COLOR) {
					float r = scanline->v.color.r * w;
					float g = scanline->v.color.g * w;
					float b = scanline->v.color.b * w;
					int R = (int)(r * 255.0f);
					int G = (int)(g * 255.0f);
					int B = (int)(b * 255.0f);
					R = CMID(R, 0, 255);
					G = CMID(G, 0, 255);
					B = CMID(B, 0, 255);
					framebuffer[x] = (R << 16) | (G << 8) | (B);
				}
				if ((render_state & RENDER_STATE_TEXTURE) || (render_state & RENDER_STATE_IMG)) {
					float u = scanline->v.tc.u * w;
					float v = scanline->v.tc.v * w;
					IUINT32 cc = device_texture_read(device, u, v);
					framebuffer[x] = cc;
				}
			}
		}
		vertex_add(&scanline->v, &scanline->step);
		if (x >= width) break;
	}
}

// ����Ⱦ����
void device_render_trap(device_t *device, trapezoid_t *trap) {
	scanline_t scanline;
	int j, top, bottom;
	top = (int)(trap->top + 0.5f);
	bottom = (int)(trap->bottom + 0.5f);
	for (j = top; j < bottom; j++) {
		if (j >= 0 && j < device->height) {
			trapezoid_edge_interp(trap, (float)j + 0.5f);
			trapezoid_init_scan_line(trap, &scanline, j);
			device_draw_scanline(device, &scanline);
		}
		if (j >= device->height) break;
	}
}

int isDelete = 0;
// ���� render_state ����ԭʼ������
void device_draw_primitive(device_t *device, vertex_t *v1,
	vertex_t *v2, vertex_t *v3) {
	point_t p1, p2, p3, c1, c2, c3;
	int render_state = device->render_state;

	direction_t light = { 0, 0.6, 0.8 };

	// ���� Transform �仯
	transform_apply(&device->transform, &c1, &v1->pos);
	transform_apply(&device->transform, &c2, &v2->pos);
	transform_apply(&device->transform, &c3, &v3->pos);

	direction_t e1 = { c3.x - c1.x, c3.y - c1.y, c3.z - c1.z };
	direction_t e2 = { c2.x - c1.x, c2.y - c1.y, c2.z - c1.z };
	direction_t n;

	vector3_crossproduct(&n, &e1, &e2);
	float sumt = n.x*n.x + n.y*n.y + n.z*n.z;
	n.x /= sqrt(sumt);
	n.y /= sqrt(sumt);
	n.z /= sqrt(sumt);

	direction_t side_D = { 0 , -1, 0 };
	direction_t main_D = { 0 , 0, 1 };
	if (n.x*main_D.x + n.y*main_D.y + n.z*main_D.z <= 0 && isDelete)	return;
	if (n.x*side_D.x + n.y*side_D.y + n.z*side_D.z <= 0 && !isDelete)	return;

	float diffuse = n.x*light.x + n.y*light.y + n.z*light.z;
	v1->color.r *= diffuse;
	v1->color.g *= diffuse;
	v1->color.b *= diffuse;
	v2->color.r *= diffuse;
	v2->color.g *= diffuse;
	v2->color.b *= diffuse;
	v3->color.r *= diffuse;
	v3->color.g *= diffuse;
	v3->color.b *= diffuse;

	// �ü���ע��˴���������Ϊ�����жϼ������� cvv���Լ�ͬcvv�ཻƽ����������
	// ���н�һ����ϸ�ü�����һ���ֽ�Ϊ������ȫ���� cvv�ڵ�������
	if (transform_check_cvv(&c1) != 0) return;
	if (transform_check_cvv(&c2) != 0) return;
	if (transform_check_cvv(&c3) != 0) return;

	// ��һ��
	transform_homogenize(&device->transform, &p1, &c1);
	transform_homogenize(&device->transform, &p2, &c2);
	transform_homogenize(&device->transform, &p3, &c3);

	// �������ɫ�ʻ���
	if (render_state & (RENDER_STATE_TEXTURE | RENDER_STATE_COLOR | RENDER_STATE_IMG)) {
		vertex_t t1 = *v1, t2 = *v2, t3 = *v3;
		trapezoid_t traps[2];
		int n;

		t1.pos = p1;
		t2.pos = p2;
		t3.pos = p3;
		t1.pos.w = c1.w;
		t2.pos.w = c2.w;
		t3.pos.w = c3.w;

		vertex_rhw_init(&t1);	// ��ʼ�� w
		vertex_rhw_init(&t2);	// ��ʼ�� w
		vertex_rhw_init(&t3);	// ��ʼ�� w

								// ���������Ϊ0-2������Ⱦ�������Σ����ҷ��ؿ���������������3�㹲����Ϊ0���߶�����x�᣿ƽ�У���Ҫ�໭1ƽ�����и�ԭ�����ͣ���Ϊ2��
		n = trapezoid_init_triangle(traps, &t1, &t2, &t3);

		if (n >= 1) device_render_trap(device, &traps[0]);
		if (n >= 2) device_render_trap(device, &traps[1]);
	}

	if (render_state & RENDER_STATE_WIREFRAME) {		// �߿����
		device_draw_line(device, (int)p1.x, (int)p1.y, (int)p2.x, (int)p2.y, device->foreground);
		device_draw_line(device, (int)p1.x, (int)p1.y, (int)p3.x, (int)p3.y, device->foreground);
		device_draw_line(device, (int)p3.x, (int)p3.y, (int)p2.x, (int)p2.y, device->foreground);
	}
}


//=====================================================================
// Win32 ���ڼ�ͼ�λ��ƣ�Ϊ device �ṩһ�� DibSection �� FB
//=====================================================================
int screen_w, screen_h, screen_exit = 0;
int screen_mx = 0, screen_my = 0, screen_mb = 0;
int screen_keys[512];	// ��ǰ���̰���״̬
static HWND screen_handle = NULL;		// ������ HWND
static HDC screen_dc = NULL;			// ���׵� HDC
static HBITMAP screen_hb = NULL;		// DIB
static HBITMAP screen_ob = NULL;		// �ϵ� BITMAP
unsigned char *screen_fb = NULL;		// frame buffer
long screen_pitch = 0;

int screen_init(int w, int h, const TCHAR *title);	// ��Ļ��ʼ��
int screen_close(void);								// �ر���Ļ
void screen_dispatch(void);							// ������Ϣ
void screen_update(void);							// ��ʾ FrameBuffer

													// win32 event handler
static LRESULT screen_events(HWND, UINT, WPARAM, LPARAM);

#ifdef _MSC_VER
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")
#endif

// ��ʼ�����ڲ����ñ���
int screen_init(int w, int h, const TCHAR *title) {
	WNDCLASS wc = { CS_BYTEALIGNCLIENT, (WNDPROC)screen_events, 0, 0, 0,
		NULL, NULL, NULL, NULL, _T("SCREEN3.1415926") };
	BITMAPINFO bi = { { sizeof(BITMAPINFOHEADER), w, -h, 1, 32, BI_RGB,
		w * h * 4, 0, 0, 0, 0 } };
	RECT rect = { 0, 0, w, h };
	int wx, wy, sx, sy;
	LPVOID ptr;
	HDC hDC;

	screen_close();

	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.hInstance = GetModuleHandle(NULL);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	if (!RegisterClass(&wc)) return -1;

	screen_handle = CreateWindow(_T("SCREEN3.1415926"), title,
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		0, 0, 0, 0, NULL, NULL, wc.hInstance, NULL);
	if (screen_handle == NULL) return -2;

	screen_exit = 0;
	hDC = GetDC(screen_handle);
	screen_dc = CreateCompatibleDC(hDC);
	ReleaseDC(screen_handle, hDC);

	screen_hb = CreateDIBSection(screen_dc, &bi, DIB_RGB_COLORS, &ptr, 0, 0);
	if (screen_hb == NULL) return -3;

	screen_ob = (HBITMAP)SelectObject(screen_dc, screen_hb);
	screen_fb = (unsigned char*)ptr;
	screen_w = w;
	screen_h = h;
	screen_pitch = w * 4;

	AdjustWindowRect(&rect, GetWindowLong(screen_handle, GWL_STYLE), 0);
	wx = rect.right - rect.left;
	wy = rect.bottom - rect.top;
	sx = (GetSystemMetrics(SM_CXSCREEN) - wx) / 2;
	sy = (GetSystemMetrics(SM_CYSCREEN) - wy) / 2;
	if (sy < 0) sy = 0;
	SetWindowPos(screen_handle, NULL, sx, sy, wx, wy, (SWP_NOCOPYBITS | SWP_NOZORDER | SWP_SHOWWINDOW));
	SetForegroundWindow(screen_handle);

	ShowWindow(screen_handle, SW_NORMAL);
	screen_dispatch();

	memset(screen_keys, 0, sizeof(int) * 512);
	memset(screen_fb, 0, w * h * 4);

	return 0;
}

int screen_close(void) {
	if (screen_dc) {
		if (screen_ob) {
			SelectObject(screen_dc, screen_ob);
			screen_ob = NULL;
		}
		DeleteDC(screen_dc);
		screen_dc = NULL;
	}
	if (screen_hb) {
		DeleteObject(screen_hb);
		screen_hb = NULL;
	}
	if (screen_handle) {
		CloseWindow(screen_handle);
		screen_handle = NULL;
	}
	return 0;
}

static LRESULT screen_events(HWND hWnd, UINT msg,
	WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_CLOSE: screen_exit = 1; break;
	case WM_KEYDOWN: screen_keys[wParam & 511] = 1; break;
	case WM_KEYUP: screen_keys[wParam & 511] = 0; break;
	default: return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

void screen_dispatch(void) {
	MSG msg;
	while (1) {
		if (!PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) break;
		if (!GetMessage(&msg, NULL, 0, 0)) break;
		DispatchMessage(&msg);
	}
}

void screen_update(void) {
	HDC hDC = GetDC(screen_handle);
	BitBlt(hDC, 0, 0, screen_w, screen_h, screen_dc, 0, 0, SRCCOPY);
	ReleaseDC(screen_handle, hDC);
	screen_dispatch();
}


//=====================================================================
// ������
//=====================================================================

void camera_at_zero(device_t *device, float x, float y, float z) {
	point_t eye = { x, y, z, 1 }, at = { 0, 0, 0, 1 }, up = { 0, 0, 1, 1 };
	matrix_set_lookat(&device->transform.view, &eye, &at, &up);
	transform_update(&device->transform);
}

void init_texture(device_t *device) {
	static IUINT32 texture[256][256];
	int i, j;
	for (j = 0; j < 256; j++) {
		for (i = 0; i < 256; i++) {
			int x = i / 32, y = j / 256;
			texture[j][i] = ((x + y) & 1) ? 0xee4866 : 0xf0c9cf;
		}
	}
	device_set_texture(device, texture, 256 * 4, 256, 256);
}

void init_img(device_t *device) {
	static IUINT32 texture[256][256];
	int i, j;

	int w, h;
	int r[65536], g[65536], b[65536];
	ImageRead(&w, &h, r, g, b);

	for (j = 0; j < 256; j++) {
		for (i = 0; i < 256; i++) {
			int t = i + j * 256;
			texture[j][i] = r[t] * 16 * 16 * 16 * 16 + g[t] * 16 * 16 + b[t];
		}
	}

	device_set_texture(device, texture, 256 * 4, 256, 256);
}

#define sqrt3 1.73205
vertex_t mesh_tetrahedron[4] = {
	{ { 0,  0,  0, 1 },{ 0, 0 },{ 1.0f, 1.0f, 1.0f }, 1 },
	{ { 0,  1,  0, 1 },{ 0, 1 },{ 1.0f, 1.0f, 1.0f }, 1 },
	{ { sqrt3 / 2,  0.5,  0, 1 },{ 1, 1 },{ 1.0f, 1.0f, 1.0f }, 1 },
	{ { sqrt3 / 6,  0.5,  sqrt3 / 3, 1 },{ 1, 0 },{ 1.0f, 1.0f, 1.0f }, 1 }
};

void draw_triangel(device_t *device, int a, int b, int c) {
	vertex_t p1 = mesh_tetrahedron[a], p2 = mesh_tetrahedron[b], p3 = mesh_tetrahedron[c];
	p1.tc.u = 0, p1.tc.v = 0;
	p2.tc.u = 0, p2.tc.v = 1;
	p3.tc.u = 1, p3.tc.v = 1;
	device_draw_primitive(device, &p1, &p2, &p3);
};

void draw_tetrahedron(device_t *device, float theta_X, float theta_Y, float theta_Z) {
	matrix_t m_X, m_Y, m_Z, m1, m2;
	matrix_set_rotate(&m_X, 1, 0, 0, theta_X);
	matrix_set_rotate(&m_Y, 0, 1, 0, theta_Y);
	matrix_set_rotate(&m_Z, 0, 0, 1, theta_Z);
	matrix_set_identity(&m1);
	matrix_set_identity(&m2);
	matrix_mul(&m1, &m_X, &m_Y);
	matrix_mul(&m2, &m1, &m_Z);
	device->transform.world = m2;
	transform_update(&device->transform);
	draw_triangel(device, 0, 1, 2);
	draw_triangel(device, 0, 3, 1);
	draw_triangel(device, 0, 2, 3);
	draw_triangel(device, 1, 3, 2);
};

int main(void)
{
	device_t device;
	int states[] = { RENDER_STATE_IMG, RENDER_STATE_TEXTURE, RENDER_STATE_COLOR, RENDER_STATE_WIREFRAME, };
	int indicator = 0;
	int kbhit = 0;
	float alpha_X = 0;
	float alpha_Y = 0;
	float alpha_Z = 0;
	float pos = 3.5;

	TCHAR *title = _T("Flower3d (software render) - ")
		_T("Q/W/A/S/Z/X: rotation, Up/Down: forward/backward, Space: switch state, Delete: backface culling");

	if (screen_init(960, 720, title))
		return -1;

	device_init(&device, 960, 720, screen_fb);
	camera_at_zero(&device, 3, 0, 0);

	init_img(&device);
	device.render_state = RENDER_STATE_IMG;

	while (screen_exit == 0 && screen_keys[VK_ESCAPE] == 0) {
		screen_dispatch();
		device_clear(&device, 1);
		camera_at_zero(&device, pos, 0, 0);

		if (screen_keys[VK_UP]) pos -= 0.01f;
		if (screen_keys[VK_DOWN]) pos += 0.01f;
		if (screen_keys[81]) alpha_X -= 0.01f;  //Q
		if (screen_keys[87]) alpha_X += 0.01f;	//W
		if (screen_keys[65]) alpha_Y -= 0.01f;  //A
		if (screen_keys[83]) alpha_Y += 0.01f;	//S
		if (screen_keys[90]) alpha_Z -= 0.01f;  //Z
		if (screen_keys[88]) alpha_Z += 0.01f;	//X

		if (screen_keys[VK_SPACE]) {
			if (kbhit == 0) {
				kbhit = 1;
				if (++indicator >= 4) indicator = 0;
				if (indicator == 0) init_img(&device);
				if (indicator == 1) init_texture(&device);
				device.render_state = states[indicator];
			}
		}
		else {
			kbhit = 0;
		}

		if (screen_keys[VK_DELETE]) {
			isDelete = 0;
		}
		else {
			isDelete = 1;
		}

		draw_tetrahedron(&device, alpha_X, alpha_Y, alpha_Z);
		draw_tetrahedron(&device, alpha_X + 3.14159265359 * 2 / 7, alpha_Y, alpha_Z);
		draw_tetrahedron(&device, alpha_X + 3.14159265359 * 4 / 7, alpha_Y, alpha_Z);
		draw_tetrahedron(&device, alpha_X + 3.14159265359 * 6 / 7, alpha_Y, alpha_Z);
		draw_tetrahedron(&device, alpha_X + 3.14159265359 * 8 / 7, alpha_Y, alpha_Z);
		draw_tetrahedron(&device, alpha_X + 3.14159265359 * 10 / 7, alpha_Y, alpha_Z);
		draw_tetrahedron(&device, alpha_X + 3.14159265359 * 12 / 7, alpha_Y, alpha_Z);
		draw_tetrahedron(&device, alpha_X + 3.14159265359 * 14 / 7, alpha_Y, alpha_Z);

		screen_update();
		Sleep(1);
	}
	return 0;
}
