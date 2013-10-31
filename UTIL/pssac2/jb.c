

#include <stdio.h>
#include <math.h>
#include "jb.h"

#define PI       M_PI
#define DEG2RAD  PI/180.0
#define RAD2DEG  180.0/PI
#define R_EARTH  6371.0
#define DD       3.0      /* Delta Degree */


/* function find
   
   Purpose: Finds the integer value within a sorted array x 
            of length n closest to value xt
   Input:   xt - Value
            x  - Sorted Array of values
            n  - Length of x
   Output:  Integer in the array of closest value to xt 
*/
int
find(float xt, float *x, int n) {
	int il, im, itst, ihalf;

//	fprintf(stdout, "%f %f\n", xt,x[n-1]);
	if(xt <= x[0]) {
		return(0);
	}
	if(xt >= x[n-1]) {
		return(n-1);
	}
	ihalf = 0;
	il = 1;
	im = n;
	itst = im-il;
//	fprintf(stdout, "%d %d %d %d\n", im, il, itst, ihalf);
	while(itst > 1) {
		ihalf = (im + il) / 2;
//		fprintf(stdout, "%f > %f\n", xt, x[ihalf-1]);
		if(xt >= x[ihalf-1]) {
//			fprintf(stdout, "Greater\n");
			il = ihalf;
		} else {
//			fprintf(stdout, "Smaller\n");
			im = ihalf;
		}
		itst = im - il;
//		fprintf(stdout, "%d %d %d %d\n", im, il, itst, ihalf);
	}
	return(il-1);
}

/* Function trd4
   
   Purpose: 

   Input:   ts    - Double Array of Travel times for distance and depth [ sec ]
            delts - Array of Distances in degrees
	    hp    - Array of Depths in Degrees
	    hs    - Specific Depth
	    delt  - Specific Distance
	    
   Output:  t     - Output Time [ sec ]
            aih   - 
            ai0   -
	    dtdd  - dt/dd
	    dtdhx - dt/dh
	    nh    - Closest Element in Depth Array
	    nd    = Closest Element in Distance Array

*/
void
trd4(JBTable *jb, float hp, float delt, 
     float *t, float *aih, float *ai0, float *dtdd, float *dtdhx) {

	int l, k;
	float dh, dd;
	float ddlt1, dtddh, ddh1, dtdhd, dtdh, dtdddh;
	float c1, c2, c3, c4;
	if(delt > jb->deltas[jb->nd-1]) {
		return;
	}
	l = find(hp, jb->depths, jb->nh);
	dh = hp - jb->depths[l];

	k = find(delt, jb->deltas, jb->nd);
	dd = delt - jb->deltas[k];
//	fprintf(stdout, "%d %d\n", l+1, k+1);
	ddlt1 = 1.0 / (jb->deltas[k+1] - jb->deltas[k]);
	dtddh = (jb->ts[l][k+1] - jb->ts[l][k]) * ddlt1;

	ddh1 = 1.0 / (jb->depths[l+1] - jb->depths[l]);
	dtdhd = (jb->ts[l+1][k] - jb->ts[l][k]) * ddh1;
	
	dtdddh = (jb->ts[l+1][k+1] - jb->ts[l+1][k] - (jb->ts[l][k+1] - jb->ts[l][k]) ) * 
		ddlt1 * ddh1;
	dtdh = dtdhd + dtdddh * dd;
	*dtdd = dtddh + dtdddh * dh;
	
	*t = jb->ts[l][k] + dtdh * dh + dtddh * dd;
	
//	fprintf(stdout, "%f %f %f %f %f\n", *dtdd, dtddh, dtdddh, dh, dd);
	c3 = *dtdd;
	*dtdd = *dtdd * RAD2DEG;
	*dtdhx = dtdh;
	c1 = -(*dtdhx) * (R_EARTH - hp);
	*aih = atan2(*dtdd, c1);
	c2 = (jb->ts[0][1] - jb->ts[0][0]) / (jb->deltas[1] - jb->deltas[0]);
	c4 = sqrt(c2*c2 - c3*c3);
//	fprintf(stdout, "%f %f %f %f\n", c1,c2,c3,c4);
	*ai0 = atan2(c3,c4);
	*aih = *aih * RAD2DEG;
	*ai0 = *ai0 * RAD2DEG;
}

float
gfact3(JBTable *jb, float dist, float depth) {
	
	int ld;
	float as, rs;
	float distr, distm1, distm2, distp1, distp2;
	float t, aih, ai0, dtdd, dtdh;
	float aihr, ai0r;
	float tm1, tm2, tp1, tp2, dum, dum2, dm, dhx;
	float p, d2tdd2, c5;;
	float gd;

	if(depth > 800) {
		return(-1.0);
	}
//	if(dist > 90 || dist < 30) {
//		return(-1.0);
//	}
	ld = find(depth, jb->z, jb->nl);
	as = jb->vp[ld]  + (jb->vp[ld+1]  - jb->vp[ld])  * 
		(depth - jb->z[ld]) / (jb->z[ld+1] - jb->z[ld]);
	rs = jb->den[ld] + (jb->den[ld+1] - jb->den[ld]) * 
		(depth - jb->z[ld]) / (jb->z[ld+1] - jb->z[ld]);
//	fprintf(stdout, "%d %d\n", ld, jb->nl);
//	fprintf(stdout, "%f\n", (depth - jb->z[ld]) / (jb->z[ld+1] - jb->z[ld]) );
//	fprintf(stdout, "%f %f\n", as, rs);
	distr  = dist * DEG2RAD;
	distm2 = dist - 2.0 * DD;
	distm1 = dist - DD;
	distp2 = dist + 2.0 * DD;
	distp1 = dist + DD;
	trd4(jb, depth, dist, &t, &aih, &ai0, &dtdd, &dtdh);
	aihr = aih * DEG2RAD;
	ai0r = ai0 * DEG2RAD;
	
	trd4(jb, depth, distm2, &tm2, &dum, &dum2, &dm, &dhx);
	trd4(jb, depth, distm1, &tm1, &dum, &dum2, &dm, &dhx);
	trd4(jb, depth, distp2, &tp2, &dum, &dum2, &dm, &dhx);
	trd4(jb, depth, distp1, &tp1, &dum, &dum2, &dm, &dhx);
	
// 	fprintf(stdout, "%f %f %f %f\n", tm2, tm1, tp2, tp1);

	dtdd = (tp1-tm1) / (2.0 * DD);
	p = dtdd * RAD2DEG;
	d2tdd2 = (tp2 - 2.0 * t + tm2 ) / (4.0 * DD * DD);
	d2tdd2 = d2tdd2 * RAD2DEG * RAD2DEG;
	c5 = rs * as * p * fabs(d2tdd2);
	c5 = c5 / (jb->vp[0] * jb->den[0] * sin(distr) * cos(aihr) * cos(ai0r));
	gd = as * sqrt(c5) / (R_EARTH - depth);
//	fprintf(stdout, "%f %f %f %f %f %f %f\n", gd, as, c5, distr, depth, aihr, ai0r);
	return(gd);
}


static JBTable JB = {
	JB_NL ,
	{0.0 , 15.0, 15.0, 33.0, 33.0, 100.0, 200.0, 300.0, 413.0, 600.0, 800.0},
	{5.57, 5.57, 6.50, 6.50, 7.75, 7.95,  8.26,  8.58,  8.97,  10.25, 11.00},
	{3.36, 3.36, 3.74, 3.74, 4.35, 4.45,  4.60,  4.76,  4.96,  5.66,  6.13 },
	{2.65, 2.65, 2.87, 2.87, 3.32, 3.38,  3.47,  3.55,  3.64,  4.13,  4.49 },
	JB_NH ,
	{0.0,   15.0,  33.0,  96.0,  160.0, 223.0, 287.0,
	 350.0, 413.0, 477.0, 540.0, 603.0, 667.0, 730.0,
	 793.0},
	JB_ND ,
	{0.0,  0.2,  0.4,  0.6,  0.8,  1.0,  1.2,  1.4,  1.6,
	 2.0,  2.5,  3.0,  3.5,  4.0,  4.5,  5.0,  5.5,  6.0,  6.5,
	 7.0,  7.5,  8.0,  8.5,  9.0,  9.5,  10.0, 11.0, 12.0, 13.0,
	 14.0, 15.0, 16.0, 17.0, 18.0, 19.0, 20.0, 21.0, 22.0, 23.0,  
	 24.0, 25.0, 26.0, 27.0, 28.0, 29.0, 30.0, 31.0, 32.0, 33.0,
	 34.0, 35.0, 36.0, 37.0, 38.0, 39.0, 40.0, 41.0, 42.0, 43.0,
	 44.0, 45.0, 46.0, 47.0, 48.0, 49.0, 50.0, 51.0, 52.0, 53.0,
	 54.0, 55.0, 56.0, 57.0, 58.0, 59.0, 60.0, 61.0, 62.0, 63.0,
	 64.0, 65.0, 66.0, 67.0, 68.0, 69.0, 70.0, 71.0, 72.0, 73.0,
	 74.0, 75.0, 76.0, 77.0, 78.0, 79.0, 80.0, 81.0, 82.0, 83.0,
	 84.0, 85.0, 86.0, 87.0, 88.0, 89.0, 90.0, 91.0, 92.0, 93.0,
	 94.0, 95.0, 96.0, 97.0, 98.0, 99.0,100.0,101.0,102.0},
	{ 
	   { 0.,       4.00000,  8.00000,  12.0000,  16.0000,  19.9000,  23.3000, 
	     26.7000,  29.6000,  35.4000,  42.6000,  49.7000,  56.8000,  63.9000,
	     71.0000,  78.1000,  85.2000,  92.2000,  99.3000,  106.300,	 113.300,
	     120.300,  127.300,  134.200,  141.100,  148.000,  161.700,  175.300,
	     188.700,  201.900,  215.000,  228.000,  240.700,  253.200,  265.500,
	     277.000,  287.400,  297.500,  307.400,  317.100,  326.800,  336.200,
	     345.400,  354.500,  363.500,  372.500,  381.300,  390.100,  398.800,
	     407.500,  416.100,  424.600,  433.000,  441.400,  449.800,  458.100,
	     466.300,  474.500,  482.700,  490.800,  498.900,  506.800,	 514.700,
	     522.600,  530.300,  538.000,  545.600,  553.200,  560.700,  568.000,  
	     575.400,  582.600,  589.800,  596.800,  603.800,  610.700,  617.500,
	     624.300,  630.900,  637.500,  644.000,  650.400,  656.800,  663.100,
	     669.300,  675.400,  681.500,  687.500,  693.400,  699.200,  705.000,
	     710.700,  716.300,  721.800,  727.300,  732.700,  738.000,  743.200,
	     748.400,  753.500,  758.500,  763.500,  768.400,  773.200,  778.000,
	     782.700,  787.300,  791.900,  796.500,  801.100,  805.700,  810.300,
	     814.800,  819.300,  823.800,  828.400,  832.900,  837.400},  

	   { 2.70000,  4.80000,  8.40000,  12.3000,  16.2000,  18.0000,  21.3000,
	     24.5000,  27.8000,  33.5000,  40.7000,  47.8000,  54.9000,  62.0000,  
	     69.1000,  76.2000,  83.2000,  90.2000,  97.3000, 104.3000,  111.300,
	     118.300,  125.300,  132.200,  139.100,  146.000,  159.700,  173.300,
	     186.700,  199.900,  213.000,  225.800,  238.500,  251.000,  263.300,  
	     274.800,  285.200,  295.300,  305.200,  314.900,  324.600,  333.800,  
	     343.000,  352.100,  361.100,  370.100,  378.900,  387.700,  396.400,  
	     405.100,  413.700,  422.100,  430.500,  438.900,  447.300,  455.600,
	     463.800,  472.000,  480.200,  488.300,  496.400,  504.300,  512.200,
	     520.100,  527.800,  535.500,  543.100,  550.700,  558.200,  565.500,
	     572.900,  580.100,  587.300,  594.300,  601.300,  608.200,  615.000,
	     621.800,  628.400,  635.000,  641.500,  647.900,  654.300,  660.600,
	     666.800,  672.900,  679.000,  685.000,  690.900,  696.700,  702.500,
	     708.100,  713.700,  719.200,  724.700,  730.100,  735.400,  740.600,
	     745.800,  750.900,  755.900,  760.900,  765.800,  770.600,  775.400,
	     780.100,  784.700,  789.300,  793.900,  798.500,  803.100,  807.700,
	     812.200,  816.700,  821.200,  825.800,  830.300,  834.800},

	   { 5.40000,  6.60000,  9.10000,  11.9000,  14.8000,  17.7000,  20.5000,
	     23.4000,  26.3000,  32.0000,  39.1000,  46.3000,  53.4000,  60.5000,
	     67.6000,  74.7000,  81.7000,  88.7000,  95.8000,  102.800,  109.800,
	     116.700,  123.700,  130.600,  137.500,  144.400,  158.100,  171.600,  
	     185.000,  198.100,  211.200,  224.100,  236.700,  249.200,  261.500,  
	     272.500,  282.900,  292.900,  302.800,  312.500,  322.200,  331.600,  
	     340.800,  349.900,  358.800,  367.700,  376.600,  385.400,  394.100,  
	     402.700,  411.300,  419.800,  428.200,  436.600,  444.900,  453.200,
	     461.500,  469.700,  477.900,  486.000,  494.000,  502.000,	 509.800,  
	     517.700,  525.400,  533.100,  540.700,  548.200,  555.700,  563.100,  
	     570.400,  577.600,  584.800,  591.800,  598.800,  605.700,  612.500,  
	     619.200,  625.900,  632.400,  638.900,  645.300,  651.600,  657.900,  
	     664.100,  670.200,	 676.300,  682.200,  688.200,  694.000,  699.800,  
	     705.500,  711.200,	 716.700,  722.200,  727.600,  732.900,  738.100,  
	     743.200,  748.300,	 753.300,  758.200,  763.100,  767.900,  772.700,  
	     777.400,  782.100,  786.700,  791.300,  795.800,  800.400,  804.900,	 
	     809.500,  814.000,  818.500,  823.100,  827.600,  832.100 },

	   {13.5000,  13.8000,  14.8000,  16.4000,  18.3000,  20.4000,  22.9000,
	    25.4000,  27.3000,  32.9000,  39.8000,  46.7000,  53.6000,  60.4000,
	    67.2000,  74.1000,  80.9000,  87.7000,  94.6000,  101.600,  108.400,
	    115.200,  122.000,  128.700,  135.500,  142.200,  155.600,  168.900,
	    182.100,  195.000,  207.900,  220.500,  232.900,  245.200,  256.500,
	    267.100,  277.300,  287.200,  296.900,  306.500,  316.200,  325.500,
	    334.700,  343.800,  352.700,  361.600,  370.400,  379.100,  387.800,
	    396.400,  404.900,  413.400,  421.800,  430.100,  438.400,  446.700,
	    454.900,  463.100,  471.300,  479.400,  487.400,  495.300,  503.200,
	    511.000,  518.700,  526.400,  534.000,  541.500,  549.000,  556.300,  
	    563.600,  570.800,  577.900,  584.900,  591.800,  598.700,  605.500,  
	    612.200,  618.900,  625.400,  631.900,  638.300,  644.600,  650.800,  
	    657.000,  663.100,  669.100,  675.100,  680.900,  686.800,  692.500,  
	    698.200,  703.800,  709.300,  714.800,  720.200,  725.500,  730.700,  
	    735.800,  740.800,  745.800,  750.700,  755.600,  760.400,  765.100,  
	    769.800,  774.500,	779.100,  783.700,  788.200,  792.800,  797.300,  
	    801.900,  806.400,  811.000,  815.500,  820.000,  824.400},

	   {21.4000,  21.6000,  22.1000,  23.1000,  24.3000,  25.8000,  27.6000,
	    29.6000,  32.0000,  36.0000,  42.1000,  48.4000,  54.8000,  61.3000,
	    67.8000,  74.4000,  81.0000,  87.6000,  94.3000,  101.000,  107.700,
	    114.300,  120.900,  127.500,  134.100,  140.600,  153.700,  166.700,
	    179.600,  192.300,  204.900,  217.300,  229.500,  241.100,  251.600,
	    261.900,  271.900,  281.700,  291.400,  301.000,  310.400,  319.700,
	    328.800,  337.900,  346.800,  355.700,  364.500,  373.200,  381.800,
	    390.300,  398.800,  407.200,  415.600,  424.000,  432.300,  440.500,
	    448.700,  456.900,  465.000,  473.000,  481.000,  488.900,  496.700,
	    504.500,  512.200,  519.800,  527.400,  534.900,  542.300,  549.600,
	    556.900,  564.100,  571.100,  578.100,  585.100,  591.900,  598.700,
	    605.400,  612.000,  618.500,  625.000,  631.400,  637.700,  643.900,
	    650.000,  656.100,  662.100,  668.000,  673.900,  679.700,  685.400,
	    691.000,  696.600,  702.100,  707.500,  712.900,  718.200,  723.400,
	    728.500,  733.500,  738.500,  743.400,  748.200,  753.000,  757.700,
	    762.400,  767.100,  771.700,  776.300,  780.800,  785.400,  789.900,
	    794.500,  799.000,  803.600,  808.100,  812.600,  817.000},

	   {29.1000,  29.5000,  29.8000,  30.3000,  31.2000,  32.3000,  33.7000,
	    35.1000,  37.0000,  40.6000,  45.7000,  51.3000,  57.1000,  63.2000,
	    69.4000,  75.6000,  82.0000,  88.3000,  94.7000,  101.100,  107.500,
	    114.000,  120.500,  126.900,  133.300,  139.700,  152.500,  165.100,
	    177.600,  190.000,  202.300,  214.400,  226.000,  236.500,  246.700,
	    256.900,  266.800,  276.500,  286.200,  295.600,  304.900,  314.100,
	    323.200,  332.200,  341.100,  349.900,  358.600,  367.300,  375.900,
	    384.400,  392.900,  401.300,  409.700,  418.000,  426.300,  434.500,
	    442.700,  450.800,  458.900,  466.900,  474.800,  482.700,  490.400,
	    498.200,  505.800,  513.400,  520.900,  528.300,  535.700,  543.000,
	    550.200,  557.300,  564.400,  571.400,  578.300,  585.100,  591.800,
	    598.500,  605.100,  611.600,  618.000,  624.400,  630.700,  636.900,
	    643.000,  649.100,  655.100,  661.100,  667.000,  672.800,  678.500,
	    684.100,  689.600,  695.100,  700.500,  705.800,  711.100,  716.200,
	    721.400,  726.400,  731.400,  736.300,  741.100,  745.800,  750.500,
	    755.200,  759.800,  764.500,  769.100,  773.600,  778.200,  782.700,
	    787.300,  791.800,  796.400,  800.900,  805.400,  809.800},

	   {36.6000,  37.0000,  37.2000,  37.5000,  38.1000,  39.1000,  40.3000,
	    41.6000,  43.2000,  45.8000,  50.3000,  55.2000,  60.4000,  66.0000,
	    71.7000,  77.5000,  83.5000,  89.5000,  95.6000,  101.800,  108.000,
	    114.200,  120.400,  126.600,  132.800,  139.000,  151.400,  163.800,
	    176.000,  188.100,  200.100,  211.300,  221.900,  232.200,  242.300,
	    252.200,  262.000,  271.700,  281.200,  290.500,  299.600,  308.700,
	    317.800,  326.800,  335.600,  344.400,  353.100,  361.700,  370.300,
	    378.800,  387.200,  395.600,  404.000,  412.300,  420.500,  428.700,
	    436.900,  445.000,  453.000,  461.000,  468.900,  476.700,  484.500,
	    492.200,  499.800,  507.300,  514.700,  522.100,  529.400,  536.600,
	    543.800,  550.900,  557.900,  564.900,  571.800,  578.600,  585.300,
	    591.900,  598.400,  604.900,  611.300,  617.600,  623.900,  630.100,
	    636.200,  642.300,  648.300,  654.300,  660.200,  666.000,  671.700,
	    677.300,  682.800,  688.300,  693.600,  698.900,  704.100,  709.300,
	    714.400,  719.400,  724.400,  729.300,  734.100,  738.800,  743.500,
	    748.200,  752.800,  757.500,  762.100,  766.600,  771.200,  775.700,
	    780.300,  784.800,  789.400,  793.900,  798.400,  802.800},

	   {43.9000,  44.0000,  44.2000,  44.5000,  45.1000,  45.9000,  46.7000,
	    47.9000,  49.2000,  51.6000,  55.4000,  59.7000,  64.4000,  69.4000,
	    74.7000,  80.1000,  85.7000,  91.4000,  97.2000,  103.100,  109.000,
	    114.900,  120.800,  126.800,  132.800,  138.800,  150.800,  162.800,
	    174.700,  186.300,  197.000,  207.600,  218.000,  228.100,  238.100,
	    247.900,  257.600,  267.200,  276.500,  285.700,  294.700,  303.800,
	    312.800,  321.700,  330.500,  339.200,  347.800,  356.400,  364.900,
	    373.400,  381.800,  390.200,  398.500,  406.800,  415.000,  423.200,
	    431.300,  439.400,  447.400,  455.400,  463.300,  471.100,  478.800,
	    486.500,  494.100,  501.600,  509.000,  516.400,  523.700,  530.900,
	    538.000,  545.100,  552.000,  558.900,  565.800,  572.500,  579.100,
	    585.700,  592.200,  598.600,  605.000,  611.300,  617.600,  623.700,
	    629.800,  635.900,  641.900,  647.800,  653.700,  659.500,  665.200,
	    670.800,  676.300,  681.700,  687.000,  692.300,  697.500,  702.700,
	    707.700,  712.800,  717.700,  722.500,  727.300,  732.100,  736.700,
	    741.400,  746.000,  750.700,  755.300,  759.800,  764.400,  769.000,
	    773.500,  778.000,  782.600,  787.100,  791.500,  795.900},
	   
	   {51.1000,  51.1000,  51.3000,  51.5000,  52.0000,  52.7000,  53.6000,
	    54.4000,  55.5000,  57.6000,  60.9000,  64.6000,  68.9000,  73.4000,
	    78.2000,  83.3000,  88.4000,  93.7000,  99.2000,  104.800,  110.400,
	    116.100,  121.800,  127.500,  133.200,  139.000,  150.600,  161.800,
	    172.900,  183.600,  194.000,  204.100,  214.200,  224.300,  234.100,
	    243.800,  253.400,  262.000,  272.000,  281.100,  290.100,  299.100,
	    308.000,  316.800,  325.500,  334.200,  342.800,  351.400,  359.900,
	    368.400,  376.800,  385.200,  393.500,  401.700,  409.900,  418.100,
	    426.200,  434.200,  442.200,  450.100,  458.000,  465.800,  473.500,
	    481.100,  488.700,  496.200,  503.600,  511.000,  518.300,  525.500,
	    532.600,  539.600,  546.500,  553.300,  560.000,  566.700,  573.300,
	    579.900,  586.400,  592.800,  599.200,  605.500,  611.700,  617.800,
	    623.900,  629.900,  635.800,  641.700,  647.500,  653.200,  658.800,
	    664.400,  669.900,  675.300,  680.600,  685.900,  691.100,  696.200,
	    701.300,  706.300,  711.200,  716.000,  720.800,  725.500,  730.200,
	    734.800,  739.400,  744.100,  748.700,  753.200,  757.800,  762.400,
	    766.900,  771.500,  776.000,  780.400,  784.800,  789.200},

	   {58.0000,  58.0000,  58.2000,  58.4000,  58.7000,  59.3000,  59.8000,
	    60.5000,  61.6000,  63.4000,  66.3000,  69.7000,  73.4000,  77.5000,
	    81.9000,  86.5000,  91.3000,  96.3000,  101.400,  106.500,  111.800,
	    117.100,  122.500,  127.900,  133.300,  138.600,  149.300,  159.900,
	    170.500,  180.900,  191.100,  201.200,  211.200,  221.100,  230.800,
	    240.300,  249.700,  259.000,  268.100,  277.100,  286.100,  295.000,
	    303.900,  312.600,  321.400,  330.000,  338.600,  347.100,  355.500,
	    363.900,  372.300,  380.600,  388.900,  397.100,  405.300,  413.400,
	    421.500,  429.500,  437.400,  445.300,  453.100,  460.800,  468.500,
	    476.100,  483.600,  491.100,  498.500,  505.800,  513.000,  520.100,
	    527.200,  534.200,  541.000,  547.800,  554.500,  561.200,  567.800,
	    574.400,  580.900,  587.300,  593.600,  599.800,  606.000,  612.100,
	    618.100,  624.100,  630.000,  635.800,  641.600,  647.300,  652.900,
	    658.400,  663.900,  669.300,  674.600,  679.800,  685.000,  690.100,
	    695.100,  700.100,  705.000,  709.800,  714.600,  719.300,  724.000,
	    728.600,  733.200,  737.800,  742.400,  747.000,  751.500,  756.000,
	    760.600,  765.100,  769.600,  774.100,  778.500,  782.900},

	   {64.5000,  64.5000,  64.7000,  64.9000,  65.2000,  65.7000,  66.1000,
	    66.7000,  67.6000,  69.2000,  71.8000,  74.8000,  78.1000,  81.7000,
	    85.7000,  89.9000,  94.3000,  98.9000,  103.600,  108.400,  113.300,
	    118.300,  123.300,  128.400,  133.500,  138.600,  148.800,  158.900,
	    169.100,  179.200,  189.200,  199.000,  208.700,  218.400,  227.900,
	    237.300,  246.500,  255.700,  264.700,  273.700,  282.600,  291.400,
	    300.200,  309.000,  317.600,  326.200,  334.700,  343.200,  351.600,
	    360.000,  368.300,  376.600,  384.800,  392.900,  401.000,  409.100,
	    417.100,  425.100,  433.000,  440.800,  448.600,  456.300,  463.900,
	    471.500,  479.000,  486.400,  493.700,  501.000,  508.100,  515.200,
	    522.200,  529.100,  536.000,  542.800,  549.500,  556.100,  562.700,
	    569.200,  575.600,  582.000,  588.300,  594.500,  600.700,  606.800,
	    612.800,  618.700,  624.600,  630.400,  636.100,  641.700,  647.300,
	    652.800,  658.200,  663.600,  668.900,  674.100,  679.300,  684.300,
	    689.400,  694.300,  699.200,  704.000,  708.700,  713.400,  718.100,
	    722.700,  727.300,  731.900,  736.500,  741.100,  745.600,  750.100,
	    754.700,  759.200,  763.700,  768.200,  772.600,  777.000},

	   {70.8000,  70.8000,  71.0000,  71.2000,  71.6000,  72.0000,  72.3000,
	    72.9000,  73.6000,  74.8000,  77.1000,  79.8000,  82.8000,  86.2000,
	    89.7000,  93.5000,  97.6000,  101.800,  106.100,  110.600,  115.200,
	    119.800,  124.500,  129.300,  134.100,  138.900,  148.700,  158.600,
	    168.500,  178.200,  187.900,  197.500,  206.900,  216.300,  225.600,
	    234.800,  243.900,  253.000,  261.900,  270.800,  279.600,  288.400,
	    297.100,  305.700,  314.300,  322.800,  331.300,  339.700,  348.100,
	    356.400,  364.700,  372.900,  381.100,  389.300,  397.400,  405.400,
	    413.400,  421.300,  429.100,  436.900,  444.600,  452.200,  459.800,
	    467.300,  474.800,  482.100,  489.400,  496.500,  503.600,  510.600,
	    517.600,  524.500,  531.300,  538.100,  544.800,  551.400,  557.900,
	    564.400,  570.800,  577.100,  583.400,  589.600,  595.700,  601.700,
	    607.700,  613.600,  619.400,  625.200,  630.900,  636.500,  642.100,
	    647.600,  653.000,  658.300,  663.600,  668.800,  673.900,  679.000,
	    684.000,  688.900,  693.800,  698.600,  703.300,  708.000,  712.700,
	    717.300,  721.900,  726.400,  731.000,  735.500,  740.000,  744.500,
	    749.000,  753.500,  758.000,  762.600,  767.000,  771.400},

	   {76.8000,  76.8000,  77.0000,  77.2000,  77.5000,  78.0000,  78.4000,
	    78.9000,  79.5000,  80.3000,  82.3000,  84.7000,  87.5000,  90.6000,
	    93.7000,  97.3000,  100.900,  104.800,  108.900,  113.100,  117.400,
	    121.700,  126.200,  130.700,  135.300,  139.900,  149.100,  158.600,
	    168.100,  177.600,  187.100,  196.400,  205.500,  214.700,  223.800,
	    232.800,  241.800,  250.800,  259.600,  268.400,  277.100,  285.800,
	    294.400,  302.900,  311.400,  319.900,  328.300,  336.700,  345.000,
	    353.300,  361.500,  369.700,  377.900,  386.000,  394.000,  402.000,
	    409.900,  417.800,  425.500,  433.300,  440.900,  448.500,  456.000,
	    463.400,  470.800,  478.100,  485.300,  492.400,  499.500,  506.500,
	    513.400,  520.200,  527.000,  533.700,  540.300,  546.900,  553.400,
	    559.800,  566.200,  572.500,  578.700,  584.900,  590.900,  597.000,
	    602.900,  608.800,  614.600,  620.300,  626.000,  631.600,  637.100,
	    642.600,  647.900,  653.300,  658.500,  663.700,  668.800,  673.900,
	    678.800,  683.800,  688.600,  693.300,  698.000,  702.700,  707.300,
	    711.900,  716.500,  721.100,  725.700,  730.300,  734.800,  739.300,
	    743.800,  748.300,  752.800,  757.300,  761.700,  766.100},

	   {82.8000,  82.8000,  83.0000,  83.1000,  83.2000,  83.6000,  84.0000,
	    84.4000,  85.0000,  85.9000,  87.7000,  89.8000,  92.3000,  95.1000,
	    98.3000,  101.400,  104.800,  108.200,  112.000,  115.900,  119.900,
	    124.000,  128.200,  132.500,  136.800,  141.200,  150.000,  159.000,
	    168.100,  177.200,  186.400,  195.500,  204.500,  213.500,  222.500,
	    231.300,  240.200,  249.000,  257.700,  266.400,  275.000,  283.600,
	    292.100,  300.600,  309.000,  317.400,  325.700,  334.100,  342.300,
	    350.500,  358.700,  366.800,  374.900,  383.000,  391.000,  398.900,
	    406.700,  414.500,  422.200,  429.800,  437.400,  444.900,  452.400,
	    459.800,  467.100,  474.400,  481.600,  488.600,  495.600,  502.600,
	    509.400,  516.200,  522.900,  529.500,  536.100,  542.600,  549.000,
	    555.400,  561.800,  568.000,  574.200,  580.300,  586.400,  592.400,
	    598.300,  604.200,  610.000,  615.700,  621.300,  626.800,  632.300,
	    637.700,  643.100,  648.300,  653.600,  658.700,  663.800,  668.800,
	    673.700,  678.600,  683.400,  688.100,  692.800,  697.500,  702.100,
	    706.700,  711.300,  715.900,  720.500,  725.100,  729.600,  734.100,
	    738.600,  743.100,  747.600,  752.100,  756.500,  760.900},

	   {88.8000,  88.8000,  89.0000,  89.1000,  89.2000,  89.4000,  89.7000,
	    90.1000,  90.6000,  91.5000,  93.0000,  94.9000,  97.2000,  99.7000,
	    102.400,  105.400,  108.600,  111.900,  115.300,  118.900,  122.600,
	    126.500,  130.500,  134.500,  138.600,  142.700,  151.200,  159.800,
	    168.500,  177.300,  186.200,  195.100,  204.000,  212.800,  221.600,
	    230.300,  239.000,  247.700,  256.300,  264.900,  273.400,  281.900,
	    290.300,  298.700,  307.100,  315.400,  323.700,  331.900,  340.100,
	    348.200,  356.300,  364.400,  372.400,  380.400,  388.300,  396.100,
	    403.800,  411.500,  419.100,  426.700,  434.200,  441.700,  449.100,
	    456.400,  463.700,  470.900,  478.000,  485.000,  491.900,  498.800,
	    505.600,  512.300,  519.000,  525.600,  532.100,  538.600,  545.000,
	    551.400,  557.600,  563.900,  570.000,  576.100,  582.100,  588.000,
	    593.900,  599.700,  605.400,  611.100,  616.600,  622.200,  627.600,
	    633.000,  638.300,  643.500,  648.700,  653.800,  658.800,  663.800,
	    668.700,  673.600,  678.400,  683.100,  687.800,  692.400,  697.000,
	    701.600,  706.200,  710.800,  715.300,  719.900,  724.400,  728.900,
	    733.500,  738.000,  742.500,  747.000,  751.400,  755.800}  
	}
};

JBTable  *
jbtable_load()  {
  return(&JB);
}
  