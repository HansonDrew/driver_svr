#include "smooth_sensor.h"
//三点线性平滑、五点线性平滑和七点线性平滑
void linearSmooth3(double in[], double out[], int N)
{
    int i;
    if (N < 3)
    {
        for (i = 0; i <= N - 1; i++)
        {
            out[i] = in[i];
        }
    }
    else
    {
        out[0] = (5.0 * in[0] + 2.0 * in[1] - in[2]) / 6.0;

        for (i = 1; i <= N - 2; i++)
        {
            out[i] = (in[i - 1] + in[i] + in[i + 1]) / 3.0;
        }

        out[N - 1] = (5.0 * in[N - 1] + 2.0 * in[N - 2] - in[N - 3]) / 6.0;
    }
}

void linearSmooth5(double in[], double out[], int N)
{
    int i;
    if (N < 5)
    {
        for (i = 0; i <= N - 1; i++)
        {
            out[i] = in[i];
        }
    }
    else
    {
        out[0] = (3.0 * in[0] + 2.0 * in[1] + in[2] - in[4]) / 5.0;
        out[1] = (4.0 * in[0] + 3.0 * in[1] + 2 * in[2] + in[3]) / 10.0;
        for (i = 2; i <= N - 3; i++)
        {
            out[i] = (in[i - 2] + in[i - 1] + in[i] + in[i + 1] + in[i + 2]) / 5.0;
        }
        out[N - 2] = (4.0 * in[N - 1] + 3.0 * in[N - 2] + 2 * in[N - 3] + in[N - 4]) / 10.0;
        out[N - 1] = (3.0 * in[N - 1] + 2.0 * in[N - 2] + in[N - 3] - in[N - 5]) / 5.0;
    }
}
//7点二次线性平滑
int LinearSmooth72(double* input, double output[],long size) {
	 
	long i(0);

	if (size < 7)
	{
		for (i = 0; i <= size - 1; i++)
		{
			output[i] = input[i];
		}
	}
	else
	{
		output[0] = (32.0 * input[0] + 15.0 * input[1] + 3.0 * input[2] - 4.0 * input[3] -
			6.0 * input[4] - 3.0 * input[5] + 5.0 * input[6]) / 42.0;
		output[1] = (5.0 * input[0] + 4.0 * input[1] + 3.0 * input[2] + 2.0 * input[3] +
			input[4] - input[6]) / 14.0;
		output[2] = (1.0 * input[0] + 3.0 * input[1] + 4.0 * input[2] + 4.0 * input[3] +
			3.0 * input[4] + 1.0 * input[5] - 2.0 * input[6]) / 14.0;
		for (i = 3; i <= size - 4; i++)
		{
			output[i] = (-2.0 * (input[i - 3] + input[i + 3]) +
				3.0 * (input[i - 2] + input[i + 2]) +
				6.0 * (input[i - 1] + input[i + 1]) + 7.0 * input[i]) / 21.0;
		}
		output[size - 3] = (1.0 * input[size - 1] + 3.0 * input[size - 2] + 4.0 * input[size - 3] +
			4.0 * input[size - 4] + 3.0 * input[size - 5] + 1.0 * input[size - 6] - 2.0 * input[size - 7]) / 14.0;
		output[size - 2] = (5.0 * input[size - 1] + 4.0 * input[size - 2] + 3.0 * input[size - 3] +
			2.0 * input[size - 4] + input[size - 5] - input[size - 7]) / 14.0;
		output[size - 1] = (32.0 * input[size - 1] + 15.0 * input[size - 2] + 3.0 * input[size - 3] -
			4.0 * input[size - 4] - 6.0 * input[size - 5] - 3.0 * input[size - 6] + 5.0 * input[size - 7]) / 42.0;
	}

	for (i = 0; i < size; i++)
	{
		input[i] = output[i];
	}

	 

	return 0;
}
void linearSmooth7(double in[], double out[], int N)
{
    int i;
    if (N < 7)
    {
        for (i = 0; i <= N - 1; i++)
        {
            out[i] = in[i];
        }
    }
    else
    {
        out[0] = (13.0 * in[0] + 10.0 * in[1] + 7.0 * in[2] + 4.0 * in[3] +
            in[4] - 2.0 * in[5] - 5.0 * in[6]) / 28.0;

        out[1] = (5.0 * in[0] + 4.0 * in[1] + 3 * in[2] + 2 * in[3] +
            in[4] - in[6]) / 14.0;

        out[2] = (7.0 * in[0] + 6.0 * in[1] + 5.0 * in[2] + 4.0 * in[3] +
            3.0 * in[4] + 2.0 * in[5] + in[6]) / 28.0;

        for (i = 3; i <= N - 4; i++)
        {
            out[i] = (in[i - 3] + in[i - 2] + in[i - 1] + in[i] + in[i + 1] + in[i + 2] + in[i + 3]) / 7.0;
        }

        out[N - 3] = (7.0 * in[N - 1] + 6.0 * in[N - 2] + 5.0 * in[N - 3] +
            4.0 * in[N - 4] + 3.0 * in[N - 5] + 2.0 * in[N - 6] + in[N - 7]) / 28.0;

        out[N - 2] = (5.0 * in[N - 1] + 4.0 * in[N - 2] + 3.0 * in[N - 3] +
            2.0 * in[N - 4] + in[N - 5] - in[N - 7]) / 14.0;

        out[N - 1] = (13.0 * in[N - 1] + 10.0 * in[N - 2] + 7.0 * in[N - 3] +
            4 * in[N - 4] + in[N - 5] - 2 * in[N - 6] - 5 * in[N - 7]) / 28.0;
    }
}

//二次函数拟合平滑。

void quadraticSmooth5(double in[], double out[], int N)
{
    int i;
    if (N < 5)
    {
        for (i = 0; i <= N - 1; i++)
        {
            out[i] = in[i];
        }
    }
    else
    {
        out[0] = (31.0 * in[0] + 9.0 * in[1] - 3.0 * in[2] - 5.0 * in[3] + 3.0 * in[4]) / 35.0;
        out[1] = (9.0 * in[0] + 13.0 * in[1] + 12 * in[2] + 6.0 * in[3] - 5.0 * in[4]) / 35.0;
        for (i = 2; i <= N - 3; i++)
        {
            out[i] = (-3.0 * (in[i - 2] + in[i + 2]) +
                12.0 * (in[i - 1] + in[i + 1]) + 17 * in[i]) / 35.0;
        }
        out[N - 2] = (9.0 * in[N - 1] + 13.0 * in[N - 2] + 12.0 * in[N - 3] + 6.0 * in[N - 4] - 5.0 * in[N - 5]) / 35.0;
        out[N - 1] = (31.0 * in[N - 1] + 9.0 * in[N - 2] - 3.0 * in[N - 3] - 5.0 * in[N - 4] + 3.0 * in[N - 5]) / 35.0;
    }
}


void quadraticSmooth7(double in[], double out[], int N)
{
    int i;
    if (N < 7)
    {
        for (i = 0; i <= N - 1; i++)
        {
            out[i] = in[i];
        }
    }
    else
    {
        out[0] = (32.0 * in[0] + 15.0 * in[1] + 3.0 * in[2] - 4.0 * in[3] -
            6.0 * in[4] - 3.0 * in[5] + 5.0 * in[6]) / 42.0;

        out[1] = (5.0 * in[0] + 4.0 * in[1] + 3.0 * in[2] + 2.0 * in[3] +
            in[4] - in[6]) / 14.0;

        out[2] = (1.0 * in[0] + 3.0 * in[1] + 4.0 * in[2] + 4.0 * in[3] +
            3.0 * in[4] + 1.0 * in[5] - 2.0 * in[6]) / 14.0;
        for (i = 3; i <= N - 4; i++)
        {
            out[i] = (-2.0 * (in[i - 3] + in[i + 3]) +
                3.0 * (in[i - 2] + in[i + 2]) +
                6.0 * (in[i - 1] + in[i + 1]) + 7.0 * in[i]) / 21.0;
        }
        out[N - 3] = (1.0 * in[N - 1] + 3.0 * in[N - 2] + 4.0 * in[N - 3] +
            4.0 * in[N - 4] + 3.0 * in[N - 5] + 1.0 * in[N - 6] - 2.0 * in[N - 7]) / 14.0;

        out[N - 2] = (5.0 * in[N - 1] + 4.0 * in[N - 2] + 3.0 * in[N - 3] +
            2.0 * in[N - 4] + in[N - 5] - in[N - 7]) / 14.0;

        out[N - 1] = (32.0 * in[N - 1] + 15.0 * in[N - 2] + 3.0 * in[N - 3] -
            4.0 * in[N - 4] - 6.0 * in[N - 5] - 3.0 * in[N - 6] + 5.0 * in[N - 7]) / 42.0;
    }
}
//三次函数拟合平滑。

/**
 * 五点三次平滑
 *
 */
void cubicSmooth5(double in[], double out[], int N)
{

    int i;
    if (N < 5)
    {
        for (i = 0; i <= N - 1; i++)
            out[i] = in[i];
    }

    else
    {
        out[0] = (69.0 * in[0] + 4.0 * in[1] - 6.0 * in[2] + 4.0 * in[3] - in[4]) / 70.0;
        out[1] = (2.0 * in[0] + 27.0 * in[1] + 12.0 * in[2] - 8.0 * in[3] + 2.0 * in[4]) / 35.0;
        for (i = 2; i <= N - 3; i++)
        {
            out[i] = (-3.0 * (in[i - 2] + in[i + 2]) + 12.0 * (in[i - 1] + in[i + 1]) + 17.0 * in[i]) / 35.0;
        }
        out[N - 2] = (2.0 * in[N - 5] - 8.0 * in[N - 4] + 12.0 * in[N - 3] + 27.0 * in[N - 2] + 2.0 * in[N - 1]) / 35.0;
        out[N - 1] = (-in[N - 5] + 4.0 * in[N - 4] - 6.0 * in[N - 3] + 4.0 * in[N - 2] + 69.0 * in[N - 1]) / 70.0;
    }
    return;
}

void cubicSmooth7(double in[], double out[], int N)
{
    int i;
    if (N < 7)
    {
        for (i = 0; i <= N - 1; i++)
        {
            out[i] = in[i];
        }
    }
    else
    {
        out[0] = (39.0 * in[0] + 8.0 * in[1] - 4.0 * in[2] - 4.0 * in[3] +
            1.0 * in[4] + 4.0 * in[5] - 2.0 * in[6]) / 42.0;
        out[1] = (8.0 * in[0] + 19.0 * in[1] + 16.0 * in[2] + 6.0 * in[3] -
            4.0 * in[4] - 7.0 * in[5] + 4.0 * in[6]) / 42.0;
        out[2] = (-4.0 * in[0] + 16.0 * in[1] + 19.0 * in[2] + 12.0 * in[3] +
            2.0 * in[4] - 4.0 * in[5] + 1.0 * in[6]) / 42.0;
        for (i = 3; i <= N - 4; i++)
        {
            out[i] = (-2.0 * (in[i - 3] + in[i + 3]) +
                3.0 * (in[i - 2] + in[i + 2]) +
                6.0 * (in[i - 1] + in[i + 1]) + 7.0 * in[i]) / 21.0;
        }
        out[N - 3] = (-4.0 * in[N - 1] + 16.0 * in[N - 2] + 19.0 * in[N - 3] +
            12.0 * in[N - 4] + 2.0 * in[N - 5] - 4.0 * in[N - 6] + 1.0 * in[N - 7]) / 42.0;
        out[N - 2] = (8.0 * in[N - 1] + 19.0 * in[N - 2] + 16.0 * in[N - 3] +
            6.0 * in[N - 4] - 4.0 * in[N - 5] - 7.0 * in[N - 6] + 4.0 * in[N - 7]) / 42.0;
        out[N - 1] = (39.0 * in[N - 1] + 8.0 * in[N - 2] - 4.0 * in[N - 3] -
            4.0 * in[N - 4] + 1.0 * in[N - 5] + 4.0 * in[N - 6] - 2.0 * in[N - 7]) / 42.0;
    }
}