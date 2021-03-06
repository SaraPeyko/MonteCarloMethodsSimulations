// TestMC.cpp
//
// C++ code to price an option
// CEV  (Constant elasticity of variance) model with a choice of parameters
// and Euler method. Given option price and number of times S hits the origin
//
//

#include "OptionData.h"
#include "NormalGenerator.h"
#include "Range.cpp"
#include <iostream>

// Defines drift + diffusion + data
namespace SDE
{
	OptionData* data;					// The data for option MC

	double drift(double t, double X)
	{
		// drift term
		return (data->r) * X;			// r*X
	}

	double diffusion(double t, double X)
	{
		// diffusion term
		double betaCEV = 1.0;
		return data->sig * pow(X, betaCEV);		// sig*X^(betaCEV)
	}
	
}

namespace Batch1
{
	double T = 0.25; 
	double K = 65; 
	double sig = 0.30; 
	double r = 0.08; 
	double S = 60;
}

namespace Batch2 
{ 
	double T = 1.0; 
	double K = 100; 
	double sig = 0.2; 
	double r = 0.0; 
	double S = 100; 
};

namespace Batch4
{	double T = 30.0; 
	double K = 100.0; 
	double sig = 0.30; 
	double r = 0.08; 
	double S = 100.0; 
};

template<typename T1, typename T2, typename T3>
double SD(T1 M, T2 r, T3 T)
{
	double SumCsquare = 0; double SumC = 0; double S = M.size();		// M = number of simulations
	typename T1::const_iterator i;

	for (i = M.begin(); i != M.end(); ++i)
	{
		SumCsquare += (*i) * (*i);
		SumC += (*i);
	}

	double para1 = sqrt(SumCsquare - ((SumC * SumC) / S));
	double para2 = para1 * exp(r * T);
	return (para2 / sqrt(S - 1));
}

template<typename T1, typename T2, typename T3>
double SE(T1 M, T2 r, T3 T)
{
	double sqrS = sqrt(M.size());
	return (SD(M, r, T) / sqrS);
}

int main()
{
	// Uncommented to test on different batches
	// using namespace Batch1;							// C = 2.13337, P = 5.84628
	// using namespace Batch2;							// C = 7.96557, P = 7.96557
	using namespace Batch4;								// C = 92.17570, P = 1.24750

	std::cout << "1 factor MC with explicit Euler\n";
	OptionData option;
	option.K = K;
	option.T = T;
	option.r = r;
	option.sig = sig;
	option.type = 1;		// Put: -1, Call: 1
	double S0 = S;


	Range<double> range(0.0, option.T);				// Create the range of time t = 0 to t = T
	double Vold = S0; 								// Assign the price S0
	double Vnew = 0;

	long N;
	std::cout << "Number of subintervals in time: ";
	std::cin >> N;
	std::vector<double> x = range.mesh(N);			// Create different value of time steps

	long NSim;
	std::cout << "Number of simulations: ";			// Create different value of simulations
	std::cin >> NSim;

	double k = option.T / double(N);
	double sqrk = sqrt(k);

	// Normal random number
	double dW;
	double price = 0.0;		// Option price

	// Random Generator
	NormalGenerator* normal = new BoostNormal();

	SDE::data = &option;

	std::vector<double> res;
	int count = 0;			// Number of times S hits origin


	for (long i = 1; i <= NSim; ++i)
	{
		// 1. Calculate path at each iteration
		if ((i / 10000) * 10000 == i)
		{
			// Give status after each 1000th iteration
			std::cout << i << std::endl;
		}

		Vold = S0;
		for (unsigned long index = 1; index < x.size(); ++index)
		{
			// 2. Create random number
			dW = normal->getNormal();

			// The FDM (in this case explicit Euler)
			Vnew = Vold + (k * SDE::drift(x[index - 1], Vold)) +
				(sqrk * SDE::diffusion(x[index - 1], Vold) * dW);

			Vold = Vnew;

			if (Vnew <= 0.0) count++;
		}

		// Calculate the price at t = T
		double tmp = option.PayOffFunction(Vnew);
		res.push_back(tmp);
		price += (tmp) / double(NSim);
	}

	// Discounting the average price
	price *= exp(-option.r * option.T);
	double STDEV = SD(res, option.r, option.T);
	double STERR = SE(res, option.r, option.T);

	// Cleanup scoped pointer
	delete normal;

	std::cout << "Price, after discounting: " << price;
	std::cout << "\nStandard Deviation: " << STDEV;
	std::cout << "\nStandard Error: " << STERR;
	std::cout << "\nNumber of time origin is hit: " << count << '\n';

	return 0;
}

