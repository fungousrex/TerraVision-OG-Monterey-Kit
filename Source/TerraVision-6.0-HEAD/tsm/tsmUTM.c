/*
 * Name: tsmUTM.c
 *
 * Description: 
 *   Routines to convert between lat/long and UTM coord systems
 *
 *   N.B. The code in this file is largely derived from the National
 *   Imagery and Mapping Agency (formerly the Defense Mapping Agency?)
 *   package, DMAMUSE 2.0. A couple of convenience functions have been
 *   provided for the tsmApi interface (end of file), and some minor
 *   prettifying of the code has been performed.
 *
 *   For further details about DMAMUSE, see http://www.nima.mil/DMAMUSE2
 *
 * Function List:
 *
 * Revision History:
 *
 *   "$Id: tsmUTM.c,v 1.12.6.1 2002/03/14 05:22:58 reddy Exp $"
 *
 * License:
 *   The contents of this file are subject to the Open Source Apache
 *   Software License Version 1.1 (the "License"); you may not use
 *   this file except in compliance with the License. You may obtain a
 *   copy of the License at http://www.tsmapi/license.shtml.
 *
 *   Software distributed under the License is distributed on an "AS IS"
 *   basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 *   License for the specific language governing rights and limitations
 *   under the License.
 *
 *   Portions are Copyright (c) SRI International, 1998-2000.
 *
 */

#include "tsmInternal.h"

/* following section taken from muse/src/lib/mdtcc/inc/cnstnts.h */

#ifndef M_PI
#define M_PI        (3.14159265358979323846) 
#endif
#define HALF_PI     (1.57079632679489661923)
#define TWO_PI      (M_PI+M_PI)

#ifndef RADDEG
#define RADDEG      (M_PI/180.0)
#endif

#define EPSILON         ( 0.0000001 )
#define DOUBLE_NULL      (-1.7976931348623158e+308) /* minimum double */

/* following section taken from muse/src/lib/mdtcc/inc/projdfn.h */

#define _NOTHER	(16)
#define NM				(cnsts->name)
#define P0				(cnsts->p0)
#define SINP0			(cnsts->sinp0)	/* SIN( ABS(P0) ) */
#define COSP0			(cnsts->cosp0)	/* COS( ABS(P0) ) */
#define L0				(cnsts->l0)
#define RADIUS			(cnsts->axis)
#define AXIS			(cnsts->axis)
#define ECC				(cnsts->e)
#define ECC2			(cnsts->e2)
#define WH				(cnsts->which)
#define NCNSTS			(cnsts->ncnsts)
#define K0				(cnsts->k0)
#define AZ0				(cnsts->a0)
#define STD1			(cnsts->p1)		
#define STD2			(cnsts->p2)
#define SLIM			(cnsts->glims[0])
#define WLIM			(cnsts->glims[1])
#define NLIM			(cnsts->glims[2])
#define ELIM			(cnsts->glims[3])
#define EQUATORIAL		( cnsts->which == 0 )
#define POLAR			( abs(cnsts->which) == 1 )

typedef struct {
  short  id;
  char   name[32];
  double l0, p0,sinp0,cosp0;
  double k0,a0, p1,p2; 
  double axis,e,e2;
  double glims[4];
  short  which, ncnsts;
  double other[_NOTHER];
} proj_dfn;

/* following section taken from muse/src/lib/mdtcc/inc/dtcc.h */

#define dtcc_allocate_ptr(n)     calloc((n),1)
#define dtcc_free_ptr(aP)        free((void*)(aP))

/* following section taken from muse/src/lib/mdtcc/stable/ellpfns.c */

static void _convertSin2468( double* u )
{
  u[0] -= u[2];
  u[1] = 2.0*u[1] - 4.0*u[3];
  u[2] *= 4.0;
  u[3] *= 8.0;
  return;
}

static double _fSin2468( double sphi, const double* u )
{
  double cos2x = 1.0 - 2.0*sphi*sphi;
  double sin2x = 2.0* sphi * sqrt( 1.0 - sphi*sphi );
  return sin2x*( u[0] + cos2x*( u[1] + cos2x*( u[2] + cos2x*u[3] )));  
}

double constN( double e )
{
  double v = sqrt(1.0-e*e);
  return ( 1.0-v )/( 1.0+v);
}

double eccentricity2( double rf )
{
  double f=( rf > EPSILON ? 1.0/rf : 0.0 );
  return f*(2.0-f);
}

void set_meridian_distance( double e, double* mdcoefs ) 
{
  if( e < EPSILON )
    mdcoefs[0] = 1.0;
  else {
    double e2 = e*e;
    double e4 = e2*e2;
    mdcoefs[0] = 1.0 - e2*( 1.0/4.0 + e2*( 3.0/64.0 + e2*(5.0/256.0) ));
    mdcoefs[1] =  -e2*( (3.0/8.0) + e2*( (3.0/32.0) + e2*(45.0/1024.0) ));
    mdcoefs[2] = e4*( (15.0/256.0) + e2*(45.0/1024.0) );
    mdcoefs[3] = -e4*e2*(35.0/3072);
    mdcoefs[4] = 0.0; 
    _convertSin2468(mdcoefs+1);
  }             
  return;
}

double meridian_distance( double phi, double sphi, const double* mdcoefs ) 
{
  if( mdcoefs[0] == 1.0 )
    return phi;
  else
    return phi*mdcoefs[0] + _fSin2468(sphi, mdcoefs+1); 
}

double vertical_radius( double e, double sphi )
{
  double esp = e*sphi;
  return 1.0/sqrt(1.0-esp*esp);  
}

void set_inverse_rectifying( double e, double* fpcoefs )
{
  if( e < EPSILON )
    fpcoefs[0] = 0.0;
  else {
    double n = constN(e);
    double n2 = n*n;
    fpcoefs[0] = n*(3.0/2.0 - n2*27.0/32.0);
    fpcoefs[1] = n2*( 21.0/16.0 -n2*55.0/32.0 );
    fpcoefs[2] = n2*n*151.0/96.0;
    fpcoefs[3] = n2*n2*1097.0/512.0;
    _convertSin2468(fpcoefs); 
  }            
  return;
}			

void inverse_rectifying_lat( double R, double sinR, const double* fpcoefs,
			     double* phi ) 
{
  if( fpcoefs[0] == 0.0 )
    *phi = R;
  else
    *phi = R + _fSin2468( sinR, fpcoefs );  
  return;
} 


/* following section taken from muse/src/lib/mdtcc/stable/projfns.c */

void sincostan( double a, double* sina, double* cosa, double* tana )
{
  if( sina ) *sina = sin(a); 
  if( cosa ) *cosa = cos(a);  
  if( tana ) *tana = tan(a);
  return;
}

double check_PI( double a )
{
  int k=2;
  while( fabs(a) > M_PI && --k >= 0 ) {
    if( a > M_PI )
      a -= TWO_PI;
    else
      if( a < -M_PI )
	a += TWO_PI;
  }
  return a;
}

double check_180( double a )
{
  if( a > 180.0 )
    a -= 360.0;
  else
    if( a < -180.0 )
      a += 360.0;
  return a;
}

short _init_std_pars ( proj_dfn* cnsts, int k, const char* name, 
		       double a, double rf, double lat0, double lon0  )
{
  if( !cnsts )
    return -1;
  else {
    int n = _NOTHER;
    cnsts->id = sizeof(proj_dfn);
    while( --n >= 0 ) cnsts->other[n] = DOUBLE_NULL;
    K0 = AZ0 = STD1 = STD2 = DOUBLE_NULL;
    ELIM = 180.0; WLIM = -180.0;NLIM = 90.0; SLIM = -90.0;
    NCNSTS = k;
    P0 = lat0*RADDEG; sincostan( P0, &SINP0, &COSP0,NULL ); 
    L0 = check_PI(lon0*RADDEG);
    AXIS = a;
    if( rf <= 0.0 )
      ECC2 = 0.0;
    else
      if( rf > 1000000.0 )
	ECC2 = 1.0 - (rf*rf)/(AXIS*AXIS); 
      else
	if( rf > 1.0 )
	  ECC2 = eccentricity2(rf);
	else
	  ECC2 = rf; 
    ECC = sqrt(ECC2); 
    WH = ( fabs(P0) < EPSILON ? 0 :
	   ( P0 > HALF_PI - EPSILON ? 1 :
	     ( P0 < -HALF_PI + EPSILON ? -1 : 2 )));
    if( name[0] == 'Z' ) {
      strcpy( NM, ( EQUATORIAL  ? "Equatorial " : 
		    ( POLAR ? "Polar " : "Oblique " )) );
      strcat( NM, name+1 );
    } else
      strcpy( NM,name );
    
    return 0;
  }
}

int projection_limit_check( const void *cnstsP, double lat, double lon )
{
  double min,max;
  int status1 = 0;
  proj_dfn* cnsts = (proj_dfn*)cnstsP;
  if( lat < SLIM || lat > NLIM ) {
    min = SLIM-2.0; max = NLIM+2.0;
    if( min < -90.0 ) min = -90.0;
    if( max > 90.0 ) max = 90.0;
    if( lat > max || lat < min )
      return -1;
    else
      status1 = 1;
  }

  if( WLIM < ELIM && lon <= ELIM && lon >= WLIM  )
    return status1;
  else
    if( WLIM > ELIM && (lon <= ELIM || lon >= WLIM) )
      return status1;
    else {
      min = check_180(WLIM-2.0); max = check_180(ELIM+2.0);
      if( min < max)
	return ( lon <= max && lon >= min ? 1 : -1 );
      else
	return ( lon <= ELIM || lon >= WLIM ? 1 : -1 ); 
    }
}

void free_projection( const void* dfnP )
{
  if( dfnP ) dtcc_free_ptr(dfnP);
}

/* following section taken from muse/src/lib/mdtcc/stable/tmfns.c: TM funcs */

#define NC				(13)
#define EP2				cnsts->other[1]
#define M0				cnsts->other[2]
#define KA				cnsts->other[3]
#define MDCFS			(cnsts->other+4)
#define RCCFS			(cnsts->other+9)
#define LABEL			"Transverse Mercator"

static double poly( double x, int n, double* c )
{
  double p = 0.0;
  while( n >= 0 ) p = p*x + c[n--];
  return p;
}

int transverse_mercator_limits(	const void* cnstsP, double* mnlat,
				double* mnlon, double* mxlat, double* mxlon )
{
  if( !cnstsP || !mnlat || !mxlat || !mnlon || !mxlon)
    return -1;
  else {
    const proj_dfn* cnsts = cnstsP;
    *mnlat = SLIM; *mxlat = NLIM; 
    *mnlon = WLIM; *mxlon = ELIM;
    return 0;
  }
}
	 
short init_transverse_mercator(	void* cnstsP, const char* opt_name, double a,
				double rf, double lat0, double lon0, double k0)
{
  if( !cnstsP )
    return -1;
  else {
    proj_dfn* cnsts = cnstsP;
    _init_std_pars( cnsts, NC, (opt_name ? opt_name : LABEL), a,rf, lat0,lon0);
    K0 = k0; EP2 = ECC2/(1.0-ECC2);
    KA = K0*AXIS; 
    /* Meridian distance coefficients */
    set_meridian_distance( ECC, MDCFS );
    /* Inverse rectifying latitude coefficients */
    set_inverse_rectifying( ECC, RCCFS );
    M0 = meridian_distance( P0, SINP0, MDCFS )*K0;
    SLIM = -89.95;
    WLIM = check_180(lon0 - 30.0);
    NLIM = 89.95;
    ELIM = check_180(lon0 + 30.0);
    return 0;
  }
}

const void* set_transverse_mercator( const char* opt_name, double a, double rf,
				     double lat0, double lon0, double k0 )
{
  void* cnsts = (void *)dtcc_allocate_ptr( sizeof(proj_dfn) );
  init_transverse_mercator( cnsts, (opt_name ? opt_name : LABEL), a, rf,
			    lat0, lon0,k0 );
  return cnsts;
}

int geo_to_transverse_mercator( const void* cnstsP, double lat, double lon,
				double* x, double* y )
{
  int status = 0;
  if( !cnstsP  )
    return -1;

  if( (status = projection_limit_check(cnstsP, lat,lon)) < 0 )
    return -2;
  else {
    double cfs[5],T2,T3,T4,T5;
    const proj_dfn *cnsts = (const proj_dfn*)cnstsP;
    double rphi = lat*RADDEG, rlam = lon*RADDEG, dlam = rlam - L0;
    double dlam2 = dlam*dlam;
    double sphi = sin(rphi), cphi = cos(rphi), tphi = tan(rphi);
    double T = tphi*tphi, G = EP2*cphi*cphi, cp2 = cphi*cphi;
    double N = AXIS*K0*vertical_radius(ECC,sphi);
    double Q = 0.5*sphi*cphi*N;
    double T1 = meridian_distance( rphi,sphi, MDCFS )*K0*AXIS;
      
    T2 = Q;
			
    Q *= (cp2/12.0);
    cfs[0] = 5.0-T; cfs[1] = 9.0; cfs[2] = 4.0;
    T3 = Q*poly(G,2,cfs);

    Q *= (cp2/30.0);
    cfs[0] = 61.0; cfs[1] = -58.0; cfs[2] = 1.0;
    cfs[0] = poly(T,2,cfs);
    cfs[1] = 270.0 - 330.0*T;
    cfs[2] = 445.0 - 680.0*T;
    cfs[3] = 324.0 - 600.0*T;
    cfs[4] =  88.0 - 192.0*T;
    T4 = Q*poly( G,4,cfs );

    Q *= (cp2/56.0);
    cfs[0] = 1385.0; cfs[1] = -3111.0; cfs[2] = 543.0; cfs[3] = -1.0;
    T5 = Q*poly(T,3,cfs);

    cfs[0] = T1; cfs[1] = T2; cfs[2] = T3; cfs[3] = T4; cfs[4] = T5;
    *y = poly( dlam2, 4, cfs ) - M0*AXIS;

    Q = N*cphi;
    T1 = Q;

    Q *= (cp2/6.0);
    T2 = Q*(1.0 - T + G);

    Q *= (cp2/20.0);
    cfs[0] = 5.0; cfs[1] = -18.0; cfs[2] = 1.0;
    cfs[0] = poly(T,2,cfs);
    cfs[1] = 14.0 - 58.0*T;
    cfs[2] = 13.0 - 64.0*T;
    cfs[3] = 4.0 - 24.0*T;
    T3 = Q*poly( G,3,cfs );

    Q *= (cp2/42.0);
    cfs[0] = 61.0; cfs[1] = -479.0; cfs[2] = 179.0; cfs[3] = -1.0;
    T4 = Q*poly(T,3,cfs);
      
    cfs[0] = T1; cfs[1] = T2; cfs[2] = T3; cfs[3] = T4;
    *x = dlam*poly( dlam2,3,cfs );

    return status;
  }
}
	
int transverse_mercator_to_geo( const void* cnstsP, double x, double y,
				double* lat, double* lon )
{
  const proj_dfn* cnsts = (const proj_dfn*)cnstsP;
  double phi1,sphi1,cphi1,tphi1, lam1, T,G,Q,K2, cfs[5], T1,T2,T3,T4, rho,v;
  double yy = y/AXIS, x2 = x*x;
  double mu = (M0 + yy/K0)/MDCFS[0];

  inverse_rectifying_lat( mu,sin(mu), RCCFS, &phi1 );
  sincostan( phi1, &sphi1, &cphi1, &tphi1 );
  T = tphi1*tphi1;
  rho = 1.0/(1.0-ECC2*sphi1*sphi1);
  rho *= AXIS*(1.0-ECC2)*sqrt(rho);
  G = EP2 * cphi1*cphi1;
  v = rho*(1.0+G);
  K2 = K0*K0;
  Q = tphi1/(2.0*rho*v*K2);
  K2 *= (v*v);

  T1 = Q;
  Q /= (12.0*K2);
  cfs[0] = 5.0 + 3.0*T; cfs[1] = 1.0 - 9.0*T; cfs[2] = -4.0;
  T2 = Q*poly(G,2,cfs);

  Q /= (30.0*K2);
  cfs[0] = 61.0; cfs[1] = 90.0; cfs[2] = 45.0;
  cfs[0] = poly(T,2,cfs);
  cfs[1] = 46.0; cfs[2] = -252.0; cfs[3] = -90.0;
  cfs[1] = poly( T,2,cfs+1 );
  cfs[2] = -3.0; cfs[3] = -66.0; cfs[4] = 225.0;
  cfs[2] = poly( T,2,cfs+2 );
  cfs[3] = 100.0 + 84.0*T;
  cfs[4] = 88.0 - 192.0*T;
  T3 = Q*poly( G,4,cfs );

  Q /= (56.0*K2);
  cfs[0] = 1385.0; cfs[1] = 3633.0; cfs[2] = 4095.0; cfs[3] = 1575.0;
  T4 = Q*poly( T,3,cfs);

  cfs[0] = -T1; cfs[1] = T2; cfs[2] = -T3; cfs[3] = T4;
  phi1 += x2*poly( x2,3,cfs ); 

  Q = 1.0/(v*cphi1*K0);
  T1 = Q;

  Q /= (6.0*K2);

  T2 = Q*(1.0 + 2.0*T + G );

  Q /= (20.0*K2);
  cfs[0] = 5.0; cfs[1] = 28.0; cfs[2] = 24.0;
  cfs[0] = poly(T,2,cfs);
  cfs[1] = 6.0 + 8.0*T;
  cfs[2] = -3.0 + 4.0*T;
  cfs[3] = -4.0 + 24.0*T;
  T3 = Q* poly(G,3,cfs);

  Q /= (42.0*K2);
  cfs[0] = 61.0; cfs[1] = 662.0; cfs[2] = 1320.0; cfs[3] = 720.0;
  T4 = Q*poly(T,3,cfs);

  cfs[0] = T1; cfs[1] = -T2; cfs[2] = T3; cfs[3] = -T4;
  lam1 = L0 + x*poly(x2,3,cfs);

  *lon = check_PI(lam1)/RADDEG;
  *lat = phi1/RADDEG;  
  return projection_limit_check( cnstsP, *lat,*lon );
}

/* tmfns.c: UTM functions */

int utm_limits(	const void* cnstsP, double* mnlat, double* mnlon,
		double* mxlat, double* mxlon )
{
  if( !mnlat || !mxlat || !mnlon || !mxlon)
    return -1;
  else {
    *mnlat = -80.0; *mxlat = 84.0; 
    *mnlon = -180.0; *mxlon = 180.0;
    return 0;
  }
}
	 
short init_utm( void* cnstsP, double a, double rf )
{
  double k0 = 0.9996;
  double lat0 = 0.0, lon0 = 0.0;
  return init_transverse_mercator( cnstsP, "Universal Transverse Mercator",
				   a,rf, lat0,lon0, k0 );
}

const void* set_utm( double a, double rf )
{
  void* cnsts = (void *)dtcc_allocate_ptr( sizeof(proj_dfn) );
  init_utm( cnsts, a,rf );
  return cnsts;
}
	
#define FN	(0.0)
#define FS	(10000000.0)
#define FE	(500000.0)

short utm_to_geo( const void* tcnsts, int zone, double easting,
		  double northing, double* lat, double* lon )
{
  const proj_dfn* cnsts = (proj_dfn*)tcnsts;
  double x = easting - FE;
  double y = ( northing < 0.0 ? -(northing+FS) : northing );
  short status = transverse_mercator_to_geo( cnsts, x,y, lat,lon );
  *lon += (zone*6-183);
  return status;
}

short geo_to_utm( const void* tcnsts, double lat, double lon, 
		  int* izone, double* east, double* north )
{
  const proj_dfn* cnsts = tcnsts;	
  short status;
  int zone = *izone;
  /* if a zone is provided use it, otherwise compute it. */
  if( zone <= 0 || zone > 60 ) {
    zone = (int)(31.0 + lon / 6.0);
    if(zone >= 61) zone = 60;	
    if(zone <= 0) zone = 1;
    *izone = zone;
  }
  lon -= (zone*6-183);		/* Change the longitude to offset */
  status = geo_to_transverse_mercator( cnsts, lat,lon, east,north );
  *east += FE;
  if( *north < 0.0 ) *north = -(*north + FS);
  return status;
}

/* The code below here is not part of the DMAMUSE package. It has been  */
/* added to provide a simple interface to the above code for the tsmApi */

#include "tsmTrace.h"

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/*
 * Synopsis:
 *   int tsmLatLongToUTM( double lat, double lon,
 *                    int *zone, double *easting, double *northing )
 *
 * Doc++ Description: *//** 
 *   Convert lat/long coordinate to UTM.
 *
 *   This function will take a (latitude,longitude) pair - both given
 *   in degrees - and will return the UTM (Universal Transverse Mercator)
 *   coord for that point (zone number, easting, and northing).
 *
 *   We use the WGS 84 ellipsoid for the UTM coordinate space.
 *
 * @param    lat the latitude (degrees). Valid range = -180..180
 * @param    lon the longitude (degrees). Valid range = -90..90 (not inclusive)
 * @param    zone (pointer to) the UTM zone number to be returned
 * @param    easting (pointer to) the UTM easting to be returned
 * @param    northing (pointer to) the UTM northing to be returned
 *
 * @return   TRUE if all went well, FALSE if an error occurred
 *
 * @author   Martin Reddy
 * @version  20 November 1997
 *
 * @see      tsmUTMToLatLong, tsmLatLongToGeocentric
 *
 */

int
tsmLatLongToUTM( double lat, double lon,
		 int *zone, double *easting, double *northing )
{
  const char *funcName = "tsmLatLongToUTM";
  const void *prjP;

  /* Check that the lat/long coordinate is valid */

  if ( lon < -180.0 || lon > 180.0 ) {
    tsmError( funcName, "Longitude out of range. Valid range = -180..180" );
    return FALSE;
  }

  if ( lat <= -90.0 || lat >= 90.0 ) {
    tsmError( funcName, "Latitude not in range -90..90 (non inclusive)" );
    return FALSE;
  }

  /* create a UTM projection structure based upon the WGS84 ellipsoid */

  if ( ( prjP = set_utm( WGS84_AXIS, WGS84_RFLAT ) ) == NULL ) {
    tsmError( funcName, "Could not initialise LatLong->UTM projection" );
    return FALSE;
  }

  *zone = 0; /* make sure that we always compute the zone number */

  /* Do the conversion */

  if ( geo_to_utm( prjP, lat, lon, zone, easting, northing ) != 0 ) {
    tsmError( funcName, "error converting to UTM" );
    free_projection( prjP );
    return FALSE;
  }

  free_projection( prjP );
  return TRUE;
}

/*
 * Synopsis:
 *   int tsmUTMToLatLong( int zone, double easting, double northing,
 *                    double *lat, double *lon )
 *
 * Doc++ Description: *//** 
 *   Convert UTM coordinate to lat/long.
 *
 *   This function will take a UTM (Universal Transverse Mercator) 
 *   coordinate and convert this into a latitude/longitude coordinate
 *   (in degrees). You specify the hemisphere by the sign on the
 *   northing value, i.e. positive northings are in the northern
 *   hemisphere whilst negative northings are in the southern hemisphere.
 *
 *   N.B. The resulting lat long values will be returned in the range:
 *   -90..+90 (latitude) and -180..+180 (longitude).
 *
 *   We use the WGS 84 ellipsoid for the UTM coordinate space.
 *
 * @param    zone the UTM zone number (1..60)
 * @param    easting the UTM easting
 * @param    northing the UTM northing
 * @param    lat (pointer to) the latitude to be returned (degrees)
 * @param    lon (pointer to) the longitude to be returned (degrees)
 *
 * @return   TRUE if all went well, FALSE if an error occurred
 *
 * @author   Martin Reddy
 * @version  20 Novemeber 1997
 *
 * @see      tsmLatLongToUTM, tsmLatLongToGeocentric
 *
 */

int
tsmUTMToLatLong( int zone, double easting, double northing,
		 double *lat, double *lon )
{
  const char *funcName = "tsmUTMToLatLong";
  const void *prjP;

  /* Check that the UTM zone number is in range */

  if ( zone < 1 || zone > 60 ) {
    tsmError( funcName, "Invalid zone number given. Must be 1..60" );
    return FALSE;
  }

  /* create a UTM projection structure based upon the WGS84 ellipsoid */

  if ( ( prjP = set_utm( WGS84_AXIS, WGS84_RFLAT ) ) == NULL ) {
    tsmError( funcName, "Could not initialise LatLong->UTM projection" );
    return FALSE;
  }

  /* Do the conversion */

  if ( utm_to_geo( prjP, zone, easting, northing, lat, lon ) != 0 ) {
    tsmError( funcName, "error converting to lat/long" );
    free_projection( prjP );
    return FALSE;
  }

  free_projection( prjP );
  return TRUE;
}

#define WGS84_A    6378137.0               /* equatorial radius */
#define WGS84_B    6356752.3142            /* polar radius */
#define WGS84_E    0.08181919092890638     /* eccentricity */
#define WGS84_E2   ( WGS84_E * WGS84_E )
#define WGS84_1_E2 ( 1.0 - WGS84_E2 )

/*
 * Synopsis:
 *   int tsmLatLongToGeocentric( double lat, double lon, double altitude,
 *                               double *x, double *y, double *z )
 *
 * Doc++ Description: *//** 
 *   Convert from Lat/Long to Geocentric coordinates.
 *
 *   This function will take a (latitude, longitude, altitude)
 *   coordinate and transform it into a geocentric coordinate. The
 *   values for latitude and longitude are assumed to be in degrees,
 *   relative to the WGS84 datum and in the range -90..+90, and
 *   -180..+180 respectively.  (N.B. you can
 *      also specify longitude in the range 0..360, where 0 is still
 *      Greenwich Meridian, but 180..360 maps to -180..0. It is not
 *      obvious that a 0..180 mapping makes sense for latitude so we
 *      do not support this.)
 *
 *   The value for altitude is assumed to be in meters above sea level
 *   (i.e. height above the geoid). If your altitude is in terms of
 *   height above the WGS84 spheroid then you should compensate for
 *   this by subtracting the geoid separation value for the region
 *   before calling this function, i.e.
 *
 *   altitude -= (double) tspCalcGeoidSeparation( lat, lon );
 *
 *   The origin of the geocentric system is the center of the
 *   spheroid. The equator is in the x-y plane and 0 degrees of
 *   longitude lies along the x-z plane.
 *
 * @param    lat the WGS84 latitude value (degrees)
 * @param    lon the WGS84 longitude value (degrees)
 * @param    altitude the altitude above the sea level (meters)
 * @param    x reference to value to contain the geocentric x coordinate
 * @param    y reference to value to contain the geocentric y coordinate
 * @param    z reference to value to contain the geocentric z coordinate
 *
 * @return   TRUE
 *
 * @author   Martin Reddy
 * @version  4 May 1998
 *
 * @see      tspGeocentricToLatLong, tsmLatLongToUTM, tsmUTMToLatLong,
 *           tspCalcGeoidSeparation
 *
 */

int
tsmLatLongToGeocentric( double lat, double lon, double altitude,
			double *x, double *y, double *z )
{
  double sinlat, n, xy;

  /* wrap around the longitude if not in default range, and */
  /* make sure that lon 180 == -180 and lon 0 == lon 360.   */

  if ( lon >= 180.0 ) lon -= 360.0;
  if ( lon == 180.0 ) lon = -180.0;

  /* apply the geoid separation value so that we are relative to sea level */

  altitude += (double) tspCalcGeoidSeparation( lat, lon );

  /* convert the degree values into radians */

  lat *= RADDEG;
  lon *= RADDEG;

  /* do the transformation into geocentric coordinates */

  sinlat = sin(lat);
  n      = WGS84_A / ( sqrt( 1.0 - ( WGS84_E2 * sinlat * sinlat ) ) );
  xy     = ( n + altitude ) * cos(lat);
  *x     = xy * cos(lon);
  *y     = xy * sin(lon);
  *z     = ( ( n * WGS84_1_E2 ) + altitude ) * sinlat;

  return TRUE;
}

/*
 * Synopsis:
 *   int tsmGeocentricToLatLong( double x, double y, double z,
 *                               double *lat, double *lon, double *altitude )
 *
 * Doc++ Description: *//** 
 *   Convert a geocentric coordinate into lat/long.
 *
 *   This function will take a geocentric (x,y,z) coordinate and
 *   convert it into a (latitude,longitude,altitude) coordinate.  The
 *   origin of the geocentric system is assumed to be at the center of
 *   the spheroid. The equator is in the x-y plane and 0 degrees of
 *   longitude lies along the x-z plane.
 *
 *   The lat/long values are returned in units of degrees relative to
 *   the WGS84 datum and in the range -90..+90, -180..+180, respectively.
 *
 *   The value for altitude is returned in units of meters above sea level
 *   (i.e. height above the geoid). If you wish altitude in terms of
 *   height above the WGS84 spheroid then you can add the geoid separation
 *   value for the region to the results obtained from this function, i.e.
 *
 *   altitude += (double) tspCalcGeoidSeparation( lat, lon );
 *
 * @param    x the geocentric x coordinate
 * @param    y the geocentric y coordinate
 * @param    z the geocentric z coordinate
 * @param    lat reference to value to contain the WGS84 latitude (degrees)
 * @param    lon reference to value to contain the WGS84 longitude (degrees)
 * @param    altitude reference to value to contain the altitude above
 *           sea level (meters)
 *
 * @return   TRUE
 *
 * @author   Martin Reddy
 * @version  5 May 1998
 *
 * @see      tspLatLongToGeocentric, tsmLatLongToUTM, tsmUTMToLatLong,
 *           tspCalcGeoidSeparation
 *
 */

int
tsmGeocentricToLatLong( double x, double y, double z,
			double *lat, double *lon, double *altitude )
{
  const double epsilon = 1e-10;
  double       d, n, phi, lambda;
  double       tan_phi_last, phi_last, tan_phi;
  double       sin_phi_last, sin_phi_last2;

  /* First treat the special case of when the distance between */
  /* the x and y coordinate is zero, i.e. at the poles.        */

  if ( ( d = sqrt( ( x * x ) + ( y * y ) ) ) < epsilon ) {

    *lat = ( z < 0.0 ) ? -90.0 : 90.0;
    *lon = 0.0;
    *altitude = fabs( z ) - WGS84_B;

  } else {

    /* Now the general case iterative solution... */

    tan_phi_last = z / d;
    phi_last     = atan2( tan_phi_last, 1.0 );

    for (;;) {

      sin_phi_last  = sin( phi_last );
      sin_phi_last2 = sin_phi_last * sin_phi_last;
      n             = WGS84_A / sqrt( 1.0 - ( WGS84_E2 * sin_phi_last2 ) );
      tan_phi       = ( z + n * WGS84_E2 * sin_phi_last ) / d;
      phi           = atan2( tan_phi, 1.0 );

      if ( fabs( phi - phi_last ) < epsilon ) break;

      tan_phi_last = tan_phi;
      phi_last     = phi;
    }

    if ( phi > M_PI / 2.0 ) phi = phi - 2.0 * M_PI;
    lambda = atan2( y, x );

    /* convert the radian lat/long results into degrees */

    *lat      = phi / RADDEG;
    *lon      = lambda / RADDEG;
    *altitude = ( d / cos( phi ) ) - n;
  }

  /* apply the geoid separation value so that we are relative to sea level */

  *altitude -= (double) tspCalcGeoidSeparation( *lat, *lon );

  return TRUE;
}
