#include "picocontroller_interface.h"
#include <fstream>
#include <string>
void main() 
{
	//r 0.971105, 0.217925, -0.093083, 0.028261 p 0.211278, 1.404860, -0.398720 v - 0.004561, 0.048281, 0.003029  av 0.173300, -0.027324, 0.377237, out  r 0.971609, 0.216577, -0.092038, 0.024476, p 0.211370, 1.403889, -0.398781 in_t 9505553881841  to_t 9505533775327
	algo_result_t p1;
	p1.pose.x = 0.211278;
	p1.pose.y = 1.404860;
	p1.pose.z = -0.398720;

	p1.pose.rw = 0.971105;
	p1.pose.rx = 0.217925;
	p1.pose.ry = -0.093083;
	p1.pose.rz = 0.028261;
	p1.vx = -0.004561;
	p1.vy = 0.048281;
	p1.vz = 0.003029;
	p1.wx = 0.173300;
	p1.wy = -0.027324;
	p1.wz = 0.377237;
	p1.pose.timestamp = 9505553881841;

	double to_t = 9505533775327;
	 
	algo_result_t p2;
	PredictMotion(p1, to_t, p2);
	
	
	std::string line;
	std::ifstream in("sensor.txt", std::ios::in);
	int substrlen = strlen("sensertest:");
	if (in)
	{
		while (getline(in, line))
		{
			std::string work_str = line;
			work_str= work_str.substr(work_str.find("sensertest:") + substrlen, work_str.length() - work_str.find("sensertest:")- substrlen);
			double w = atof (work_str.substr(0, work_str.find_first_of(",")).c_str());
			work_str = work_str.substr(work_str.find_first_of(",")+1, work_str.length()- work_str.find_first_of(",")-1);
			double x= atof(work_str.substr(0, work_str.find_first_of(",")).c_str());
			work_str = work_str.substr(work_str.find_first_of(",") + 1, work_str.length() - work_str.find_first_of(",") - 1);
			double y = atof(work_str.substr(0, work_str.find_first_of(",")).c_str());
			work_str = work_str.substr(work_str.find_first_of(",") + 1, work_str.length() - work_str.find_first_of(",") - 1);
			double z = atof(work_str.substr(0, work_str.find_first_of(",")).c_str());
			work_str = work_str.substr(work_str.find_first_of(",") + 1, work_str.length() - work_str.find_first_of(",") - 1);

			double px = atof(work_str.substr(0, work_str.find_first_of(",")).c_str());
			work_str = work_str.substr(work_str.find_first_of(",") + 1, work_str.length() - work_str.find_first_of(",") - 1);
			double py = atof(work_str.substr(0, work_str.find_first_of(",")).c_str());
			work_str = work_str.substr(work_str.find_first_of(",") + 1, work_str.length() - work_str.find_first_of(",") - 1);
            double pz = atof(work_str.substr(0, work_str.find_first_of(",")).c_str());
			work_str = work_str.substr(work_str.find_first_of(",") + 1, work_str.length() - work_str.find_first_of(",") - 1);


			double wl = atof(work_str.substr(0, work_str.find_first_of(",")).c_str());
			work_str = work_str.substr(work_str.find_first_of(",") + 1, work_str.length() - work_str.find_first_of(",") - 1);
			double xl = atof(work_str.substr(0, work_str.find_first_of(",")).c_str());
			work_str = work_str.substr(work_str.find_first_of(",") + 1, work_str.length() - work_str.find_first_of(",") - 1);
			double yl = atof(work_str.substr(0, work_str.find_first_of(",")).c_str());
			work_str = work_str.substr(work_str.find_first_of(",") + 1, work_str.length() - work_str.find_first_of(",") - 1);
			double zl = atof(work_str.substr(0, work_str.find_first_of(",")).c_str());
			work_str = work_str.substr(work_str.find_first_of(",") + 1, work_str.length() - work_str.find_first_of(",") - 1);

			double pxl = atof(work_str.substr(0, work_str.find_first_of(",")).c_str());
			work_str = work_str.substr(work_str.find_first_of(",") + 1, work_str.length() - work_str.find_first_of(",") - 1);
			double pyl = atof(work_str.substr(0, work_str.find_first_of(",")).c_str());
			work_str = work_str.substr(work_str.find_first_of(",") + 1, work_str.length() - work_str.find_first_of(",") - 1);
			double pzl = atof(work_str.substr(0, work_str.find_first_of(",")).c_str());
			work_str = work_str.substr(work_str.find_first_of(",") + 1, work_str.length() - work_str.find_first_of(",") - 1);



			double wr = atof(work_str.substr(0, work_str.find_first_of(",")).c_str());
			work_str = work_str.substr(work_str.find_first_of(",") + 1, work_str.length() - work_str.find_first_of(",") - 1);
			double xr = atof(work_str.substr(0, work_str.find_first_of(",")).c_str());
			work_str = work_str.substr(work_str.find_first_of(",") + 1, work_str.length() - work_str.find_first_of(",") - 1);
			double yr = atof(work_str.substr(0, work_str.find_first_of(",")).c_str());
			work_str = work_str.substr(work_str.find_first_of(",") + 1, work_str.length() - work_str.find_first_of(",") - 1);
			double zr = atof(work_str.substr(0, work_str.find_first_of(",")).c_str());
			work_str = work_str.substr(work_str.find_first_of(",") + 1, work_str.length() - work_str.find_first_of(",") - 1);

			double pxr = atof(work_str.substr(0, work_str.find_first_of(",")).c_str());
			work_str = work_str.substr(work_str.find_first_of(",") + 1, work_str.length() - work_str.find_first_of(",") - 1);
			double pyr = atof(work_str.substr(0, work_str.find_first_of(",")).c_str());
			work_str = work_str.substr(work_str.find_first_of(",") + 1, work_str.length() - work_str.find_first_of(",") - 1);
			double pzr = atof(work_str.substr(0, work_str.find_first_of(",")).c_str());
			work_str = work_str.substr(work_str.find_first_of(",") + 1, work_str.length() - work_str.find_first_of(",") - 1);

			int64_t ts = atoll(work_str.substr(0, work_str.find_first_of(",")).c_str());
			printf("%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf%,%lld\n",
				w,x,y,z,px,py,pz,
				wl,xl,yl,zl,pxl,pyl,pzl,
				wr ,xr, yr, zr, pxr, pyr, pzr,
				ts);
		}
	}
}