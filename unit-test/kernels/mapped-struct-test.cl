
//#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#pragma OPENCL EXTENSION cl_amd_fp64 : enable


struct Data {
	int    m_n;
	float  m_x;
	double m_y;
	uint   m_d[3];
};


__kernel
void mappedStructTest(__global struct Data *data)
{
	data->m_n *= 2;

	float x = data->m_x;
	x = x * x;
	data->m_x = x;

	data->m_y *= 4;

	uint sum = 0;
	sum += data->m_d[0];
	sum += data->m_d[1];
	sum += data->m_d[2];

	data->m_d[2] = sum;
}
