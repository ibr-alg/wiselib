// vim: set noexpandtab ts=4 sw=4:

#ifndef STANDALONE_MATH_H
#define STANDALONE_MATH_H

#include <stdint.h>

namespace wiselib {

template<typename OsModel_P>
class StandaloneMath {
	public:
      typedef OsModel_P OsModel;
		typedef double real_t;
		typedef int32_t integer_t;
		
		static const real_t PI, PI_2, PI_3, PI_4, PI_6;
		
		static real_t sqrt(real_t);
		
		// ---- Trigonometry
		
		static real_t radians_to_degrees(real_t);
		static real_t degrees_to_radians(real_t);
		
		static real_t sin(real_t);
		static real_t cos(real_t);
		static real_t tan(real_t);
		
		static real_t asin(real_t);
		static real_t acos(real_t);
		
		// ---- Rounding & signs
		
		static real_t fabs(real_t);
		
		template<typename T>
		static T sgn(T x) {
			if(x > 0) { return 1; }
			else if(x < 0) { return -1; }
			return 0;
		}
		
		/// chop off everything after the decimal dot
		static integer_t trunc(real_t);
		/// round to nearest integer that is <= value
		static integer_t floor(real_t);
		/// round to nearest integer that is >= value
		static integer_t ceil(real_t);
		/// round to nearest integer
		static integer_t round(real_t);
		
	private:
		static real_t sin_degrees(integer_t);
		static real_t cos_degrees(integer_t);
		static integer_t asin_degrees(real_t);
		
		static real_t sqrt_eps(real_t, real_t);
		
		static const real_t sin_table_[91];
};

template<typename OsModel_P>
const typename StandaloneMath<OsModel_P>::real_t StandaloneMath<OsModel_P>::PI = 3.14159265358979323846; /* pi */

template<typename OsModel_P>
const typename StandaloneMath<OsModel_P>::real_t StandaloneMath<OsModel_P>::PI_2 = 1.57079632679489661923; /* pi/2 */

template<typename OsModel_P>
const typename StandaloneMath<OsModel_P>::real_t StandaloneMath<OsModel_P>::PI_3 = 1.04719755119659763132; /* pi/3 */

template<typename OsModel_P>
const typename StandaloneMath<OsModel_P>::real_t StandaloneMath<OsModel_P>::PI_4 = 0.78539816339744830962; /* pi/4 */

template<typename OsModel_P>
const typename StandaloneMath<OsModel_P>::real_t StandaloneMath<OsModel_P>::PI_6 = 0.52359877559829881566; /* pi/6 */

template<typename OsModel_P>
const typename StandaloneMath<OsModel_P>::real_t StandaloneMath<OsModel_P>::sin_table_[91] = {
	0.000000000000000000000000000000,
	0.017452406437283511653202339176,
	0.034899496702500969191884649945,
	0.052335956242943834637593170100,
	0.069756473744125302438590097154,
	0.087155742747658165869850677154,
	0.104528463267653470847307062286,
	0.121869343405147476100403025612,
	0.139173100960065437847745783984,
	0.156434465040230868959625354364,
	0.173648177666930331186634361984,
	0.190808995376544804356555573577,
	0.207911690817759342575499204031,
	0.224951054343865003426472526371,
	0.241921895599667730047954705697,
	0.258819045102520739476403832668,
	0.275637355816999163327096766807,
	0.292371704722736769355151409400,
	0.309016994374947395751718204338,
	0.325568154457156699876918537484,
	0.342020143325668712908083080038,
	0.358367949545300268354708350671,
	0.374606593415912014766178117497,
	0.390731128489273771275946955939,
	0.406736643075800208269043878317,
	0.422618261740699441286750470681,
	0.438371146789077403838064128649,
	0.453990499739546748969587497413,
	0.469471562785890805802324621254,
	0.484809620246337058535601727272,
	0.499999999999999944488848768742,
	0.515038074910054155530758634995,
	0.529919264233204900804707904172,
	0.544639035015027084263294909761,
	0.559192903470746904837085367035,
	0.573576436351046048400803556433,
	0.587785252292473137103456792829,
	0.601815023152048267363056766044,
	0.615661475325658291701813595864,
	0.629320391049837390973209494405,
	0.642787609686539251896419955301,
	0.656059028990507275835852851742,
	0.669130606358858237570075289113,
	0.681998360062498476530379321048,
	0.694658370458997254104360763449,
	0.707106781186547461715008466854,
	0.719339800338651080835461470997,
	0.731353701619170459835572728480,
	0.743144825477394244117590460519,
	0.754709580222772014046483946004,
	0.766044443118978013451680908474,
	0.777145961456970901792828954058,
	0.788010753606722014197316639184,
	0.798635510047292829227671973058,
	0.809016994374947451262869435595,
	0.819152044288991798559607104835,
	0.829037572555041735178349426860,
	0.838670567945424050293468098971,
	0.848048096156425956770874563517,
	0.857167300702112333610216410307,
	0.866025403784438596588302061718,
	0.874619707139395741180010190874,
	0.882947592858926877390501886111,
	0.891006524188367787786546614370,
	0.898794046299167037616939524014,
	0.906307787036649936673882166360,
	0.913545457642600866599025266623,
	0.920504853452440374717014037742,
	0.927183854566787424289486807538,
	0.933580426497201742996878692793,
	0.939692620785908316882739654829,
	0.945518575599316735136312672694,
	0.951056516295153531181938433292,
	0.956304755963035435506469639222,
	0.961261695938318894150143023580,
	0.965925826289068312213714762038,
	0.970295726275996472942608761514,
	0.974370064785235245885530730447,
	0.978147600733805577810642262193,
	0.981627183447663975712771389226,
	0.984807753012208020315654266597,
	0.987688340595137770350220307591,
	0.990268068741570361979142944620,
	0.992546151641321983127852490725,
	0.994521895368273289861349439889,
	0.996194698091745545198705258372,
	0.997564050259824197652847033169,
	0.998629534754573833232882407174,
	0.999390827019095762118183756684,
	1.000000000000000000000000000000
};

template<typename OsModel_P>
typename StandaloneMath<OsModel_P>::real_t
StandaloneMath<OsModel_P>::
fabs(typename StandaloneMath<OsModel_P>::real_t x) {
	if(x < 0) { return -x; }
	return x;
}

template<typename OsModel_P>
typename StandaloneMath<OsModel_P>::real_t
StandaloneMath<OsModel_P>::
sqrt_eps(typename StandaloneMath<OsModel_P>::real_t x, typename StandaloneMath<OsModel_P>::real_t epsilon) {
	//static const integer_t max_iterations = 1000;
	typename StandaloneMath<OsModel_P>::integer_t iterations = 0;
	
	if(x < 0) { // || x <= (2.0*epsilon*epsilon)) {
		return -1.0;
	}
	
	typename StandaloneMath<OsModel_P>::real_t y = x;
	while(fabs((y*y) - x) > epsilon /*&& iterations < max_iterations*/) {
		y = (y + (x / y)) / 2.0;
		iterations++;
	}
	return y;
}

template<typename OsModel_P>
typename StandaloneMath<OsModel_P>::real_t
StandaloneMath<OsModel_P>::
sqrt(typename StandaloneMath<OsModel_P>::real_t x) {
	return sqrt_eps(x, 0.00000001);
}

// Degrees <-> Radians conversion
template<typename OsModel_P>
typename StandaloneMath<OsModel_P>::real_t
StandaloneMath<OsModel_P>::
radians_to_degrees(typename StandaloneMath<OsModel_P>::real_t x) { return 180.0 * x / PI; }

template<typename OsModel_P>
typename StandaloneMath<OsModel_P>::real_t
StandaloneMath<OsModel_P>::
degrees_to_radians(typename StandaloneMath<OsModel_P>::real_t x) { return PI * x / 180.0; }

// Basic trigonometric functions
template<typename OsModel_P>
typename StandaloneMath<OsModel_P>::real_t
StandaloneMath<OsModel_P>::
sin(typename StandaloneMath<OsModel_P>::real_t x) {
	return sin_degrees(round(radians_to_degrees(x)));
}

template<typename OsModel_P>
typename StandaloneMath<OsModel_P>::real_t
StandaloneMath<OsModel_P>::
cos(typename StandaloneMath<OsModel_P>::real_t x) {
	return sin(x + PI_2);
}

template<typename OsModel_P>
typename StandaloneMath<OsModel_P>::real_t
StandaloneMath<OsModel_P>::
tan(typename StandaloneMath<OsModel_P>::real_t x) {
	return sin(x) / cos(x);
}

template<typename OsModel_P>
typename StandaloneMath<OsModel_P>::real_t
StandaloneMath<OsModel_P>::
sin_degrees(typename StandaloneMath<OsModel_P>::integer_t x) {
	int y = x % 360;
	if(y < 0) { y += 360; }
	
	if(y < 90) { return sin_table_[y]; }
	else if(y < 180) { return sin_table_[180 - y]; }
	else if(y < 270) { return -sin_table_[y - 180]; }
	else { return -sin_table_[360 - y]; }
}

template<typename OsModel_P>
typename StandaloneMath<OsModel_P>::real_t
StandaloneMath<OsModel_P>::
cos_degrees(typename StandaloneMath<OsModel_P>::integer_t x) {
	return sin_degrees(x + 90);
}

template<typename OsModel_P>
typename StandaloneMath<OsModel_P>::real_t
StandaloneMath<OsModel_P>::
asin(typename StandaloneMath<OsModel_P>::real_t x) {
	return degrees_to_radians(asin_degrees(x));
}

template<typename OsModel_P>
typename StandaloneMath<OsModel_P>::real_t
StandaloneMath<OsModel_P>::
acos(typename StandaloneMath<OsModel_P>::real_t x) {
	return degrees_to_radians(180 + asin_degrees(-x));
}

template<typename OsModel_P>
typename StandaloneMath<OsModel_P>::integer_t
StandaloneMath<OsModel_P>::
asin_degrees(typename StandaloneMath<OsModel_P>::real_t x) {
	if(x < 0) {
		return -asin_degrees(-x);
	}
	
	// sin_table_ is monotone, conduct a binary search
	integer_t top = 90, bottom = 0, idx = 0;
	while(top - bottom > 1) {
		idx = (top + bottom) / 2;
		if(sin_table_[idx] > x) {
			top = idx;
		}
		else {
			bottom = idx;
		}
	}
	
	idx = bottom;
	if(
		fabs(sin_table_[top] - x) <
		fabs(sin_table_[bottom] - x)
	) {
		idx = top;
	}
	return idx;
}


// Rounding
template<typename OsModel_P>
typename StandaloneMath<OsModel_P>::integer_t
StandaloneMath<OsModel_P>::
trunc(typename StandaloneMath<OsModel_P>::real_t x) {
	return static_cast<integer_t>(x);
}

template<typename OsModel_P>
typename StandaloneMath<OsModel_P>::integer_t
StandaloneMath<OsModel_P>::
floor(typename StandaloneMath<OsModel_P>::real_t x) {
	if(x > 0) { return trunc(x); }
	else {
		//   floor(-12.3456) = -13
		//   trunc(-12.3456) = -12
		if(static_cast<real_t>(trunc(x)) != x) {
			return trunc(x) - 1;
		}
		else {
			return trunc(x);
		}
	}
}

template<typename OsModel_P>
typename StandaloneMath<OsModel_P>::integer_t
StandaloneMath<OsModel_P>::
ceil(typename StandaloneMath<OsModel_P>::real_t x) {
	return -floor(-x);
}

template<typename OsModel_P>
typename StandaloneMath<OsModel_P>::integer_t
StandaloneMath<OsModel_P>::
round(typename StandaloneMath<OsModel_P>::real_t x) {
	return floor(x + 0.5);
}

} // namespace wiselib

#endif // STANDALONE_MATH_H

