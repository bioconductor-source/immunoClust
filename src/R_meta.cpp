/*
 *  R_meta.cpp
 *  
 *
 *  Created by till on 10/14/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "em_meta.h"
#include "hc_meta.h"

#include "meta_scale.h"
#include "meta_gpa.h"
#include "normalize.h"


#include <R.h>
#include <Rinternals.h>
#include <R_ext/Print.h>

#ifdef __cplusplus
extern "C" {
#endif	
	
	
    /*
	// metaME
	void metaME(int* n, int* p, int* g, double* w, double* m, double* s,
				double* z, double* gw, double* gm, double* gs, 
				int* label, double* logLike, int* history, int* B, double* tol, int* method, double* bias, double* alpha, int* min_g)
	{
		int status = 0;
		em_meta em(*n, *p, *g, w, m, s, z, gw, gm, gs, *bias, *alpha);
		switch(*method) {
			case 1:		//
				em.start(label, false);
				status = em.bc_maximize(*B, *tol);
				break;
			case 2:		// EM-T: classification no weights
				em.start(label, false);
				status = em.do_classify(*B, *tol, *min_g);
				break;
			case 10:
				em.start(label, true);
				status = em.bc_maximize(*B, *tol);
				break;
			case 20:	// EM-T: classification with weights
				em.start(label, true);
				status = em.do_classify(*B, *tol, *min_g);
				break;
			default:
				em.start(label, false);
				status = em.kl_minimize(*B, *tol);
				break;
		}
		*g = em.final(label, logLike, history);
		Rprintf("The EM (%d) with %d clusters required %d iterations, has tolerance %g and loglike %g\n",status, *g, *B, *tol, logLike[1]);	

	}
     */
	
	
	
	// metaScale
	void metaScale(int* p, int* n, int* k, double* w, double* m, double* s, int* label, int* method)
	{
		meta_scale scale(*p, *n, k, w, m, s);
		switch(*method) {
			case 0:
			default:	// sample median absolute deviation => averaged median absolute deviation
				scale.mad();
				break;
			case 2:	// sample trimed mean/sd => averaged mean, averaged sd
				scale.trm(0.9);
				break;
			case 1:	// sample trimed sd (mean=0!) => averaged trimed sd
				scale.trm0(0.9);
				break;
			case 3:	// sample trimed mean/sd => mean=0, sd=1
				scale.trm_c(0.9);
				break;
			case 4:	// weighted mean/td => averaged mean,sd
				scale.trm_w();
				break;
            case 5: // scale by (averaged 0.9-quantiles of sample means / 0.9-quantile of sample means)
                scale.quantile();
                break;
		}
	}

    //metaGPA
	void metaGPA(int* p, int* n, int* k, double* w, double* m, double* s, int* g, double* z, int* groups)
	{
		meta_gpa normalizer(*p, *n, k, w, m, s, *g, z);
		normalizer.process();
	}
	void metaGPAl(int* p, int* n, int* k, double* w, double* m, double* s, int* label, int* groups)
	{
		meta_gpa normalizer(*p, *n, k, w, m, s, label);
		normalizer.process();
	}
    
    
	//metaNormalize
	void metaNormalize(int* p, int* n, int* k, double* w, double* m, double* s, int* g, double* z, int* degree)
	{
		normalize normalizer(*p, *n, k, w, m, s, *g, z, 0, *degree);
		normalizer.process();
	}
	
	void metaHC(int* li, int* lj, double* crit, int* k, int* p, double* w, double* m, double* s)
	{
		mvn_dendro dendro(*k, *p, w, m, s);
		dendro.hellinger(li, lj, crit);
	}
    
    
	void mvnDendro(int* li, int* lj, double* crit, int* k, int* p, double* w, double* m, double* s, int* method )
	{
		//Rprintf("mvnDendro %d\n", *method);
		mvn_dendro dendro(*k, *p, w, m, s);
		if( *method==0 ) {
			dendro.hellinger_d(li, lj, crit);
		}
		else
            if(*method==1 ) {
                dendro.hellinger(li, lj, crit);
            }
            else 
                if(*method==2 ) {
                    dendro.hellinger_w(li, lj, crit);
                }
                else {
                    dendro.mahalanobis(li, lj, crit);
                }
		
	}
    
    
    //
    // call methods
    //
    
    static SEXP _ME_ret(int n, int p, int k) 
	{
        
		SEXP ret = Rf_protect(allocVector(VECSXP, 11));
		SEXP names = Rf_protect(allocVector(STRSXP, 11));
		
		SET_STRING_ELT(names, 0, mkChar("L"));
		SET_STRING_ELT(names, 1, mkChar("z"));
		SET_STRING_ELT(names, 2, mkChar("w"));
		SET_STRING_ELT(names, 3, mkChar("m"));
		SET_STRING_ELT(names, 4, mkChar("s"));
		SET_STRING_ELT(names, 5, mkChar("label"));
		SET_STRING_ELT(names, 6, mkChar("logLike"));
		SET_STRING_ELT(names, 7, mkChar("history"));
		SET_STRING_ELT(names, 8, mkChar("status"));
		SET_STRING_ELT(names, 9, mkChar("iterations"));
		SET_STRING_ELT(names, 10, mkChar("tolerance"));
		
		SET_VECTOR_ELT(ret, 0, allocVector(INTSXP, 1));		// out L
		SET_VECTOR_ELT(ret, 1, allocVector(REALSXP, n*k));	// out z (!not initialzed!)
		SET_VECTOR_ELT(ret, 2, allocVector(REALSXP, k));	// out w
		SET_VECTOR_ELT(ret, 3, allocVector(REALSXP, k*p));	// out m
		SET_VECTOR_ELT(ret, 4, allocVector(REALSXP, k*p*p));// out s
		SET_VECTOR_ELT(ret, 5, allocVector(INTSXP, n));		// out label
		SET_VECTOR_ELT(ret, 6, allocVector(REALSXP, 3));	// out logLike
		SET_VECTOR_ELT(ret, 7, allocVector(INTSXP, k));		// out history
		SET_VECTOR_ELT(ret, 8, allocVector(INTSXP, 1));		// out status
		SET_VECTOR_ELT(ret, 9, allocVector(INTSXP, 1));		// out iteratioms
		SET_VECTOR_ELT(ret, 10, allocVector(REALSXP, 1));	// out tollerance
		
    	setAttrib(ret, R_NamesSymbol, names);
		
		Rf_unprotect(1);	// unproctedt names
		
		return ret;
		
	}
    
    // metaME
	SEXP call_metaME(SEXP N, SEXP P, SEXP K, SEXP W, SEXP M, SEXP S,
                     SEXP label, SEXP max_iter, SEXP max_tol, SEXP method, 
                     SEXP bias, SEXP alpha, SEXP min_g)
	{
		int status = 0;
       
        int iterations = INTEGER(max_iter)[0];
        double tolerance = REAL(max_tol)[0];
        
        SEXP ret = _ME_ret(INTEGER(N)[0], INTEGER(P)[0], INTEGER(K)[0]);
        
		em_meta em(INTEGER(N)[0], INTEGER(P)[0], INTEGER(K)[0], 
                   REAL(W), REAL(M), REAL(S), 
                   REAL(VECTOR_ELT(ret,1)), REAL(VECTOR_ELT(ret,2)), 
                   REAL(VECTOR_ELT(ret,3)), REAL(VECTOR_ELT(ret,4)), 
                   REAL(bias)[0], REAL(alpha)[0]);
        
		switch(INTEGER(method)[0]) {
			case 1:		//
				em.start(INTEGER(label), false);
				status = em.bc_maximize(iterations, tolerance);
				break;
			case 2:		// EM-T: classification no weights
				em.start(INTEGER(label), false);
				status = em.do_classify(iterations, tolerance, INTEGER(min_g)[0]);
				break;
			case 10:
				em.start(INTEGER(label), true);
				status = em.bc_maximize(iterations, tolerance);
				break;
			case 20:	// EM-T: classification with weights
				em.start(INTEGER(label), true);
				status = em.do_classify(iterations, tolerance, INTEGER(min_g)[0]);
				break;
			default:
				em.start(INTEGER(label), false);
				status = em.kl_minimize(iterations, tolerance);
				break;
		}
		INTEGER(VECTOR_ELT(ret,8))[0] = status;		
		INTEGER(VECTOR_ELT(ret,9))[0] = iterations;
		REAL(VECTOR_ELT(ret,10))[0] = tolerance;
        
		INTEGER(VECTOR_ELT(ret,0))[0] = em.final(INTEGER(VECTOR_ELT(ret,5)), 
                                                 REAL(VECTOR_ELT(ret,6)), 
                                                 INTEGER(VECTOR_ELT(ret,7)) );
		Rprintf("The EM (%d) with %d clusters required %d iterations, has tolerance %g and loglike %g\n",
                status, INTEGER(VECTOR_ELT(ret,0))[0], iterations, 
                tolerance, REAL(VECTOR_ELT(ret,6))[0]);	
                                                 
        Rf_unprotect(1);	// unprocted ret
                                                 
        return ret;
        
	}
    
	
	
#ifdef __cplusplus
}
#endif
