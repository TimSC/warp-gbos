

#include "OSTN02Perl.h"
extern "C"
{
	#include <EXTERN.h> /* from the Perl distribution */
	#include <perl.h> /* from the Perl distribution */
}

OSTN02Perl::OSTN02Perl()
{
	//char *args[] = { NULL };
	int argc = 0;
	char **argv = 0;
	char **env = 0;
	PERL_SYS_INIT3(&argc,&argv,&env);
	my_perl = perl_alloc();
	perl_construct(my_perl);

	const char *scripts[] = {"","OSTN02.pl",0};
	perl_parse(my_perl, NULL, 0, (char **)scripts, NULL);
	PL_exit_flags |= PERL_EXIT_DESTRUCT_END;

}

OSTN02Perl::~OSTN02Perl()
{
	perl_destruct(my_perl);
	perl_free(my_perl);
	PERL_SYS_TERM();
}

void OSTN02Perl::ConvertGbos1936ToWgs84(double ea, double no, double he,
	double &latOut, double &lonOut, double &heOut)
{
	//const char *scripts2[] = {"","OSTN02gridtolatlon.pl",0};
	//perl_parse(my_perl, NULL, 0, (char **)scripts2, NULL);
	//PL_exit_flags |= PERL_EXIT_DESTRUCT_END;

	string cmd = "$a = OSTN02gridtolatlonstr(";
	cmd += ToString<double>(ea);
	cmd += ",";
	cmd += ToString<double>(no);
	cmd += ");";

	eval_pv(cmd.c_str(), TRUE);
	string retStr = SvPV_nolen(get_sv("a", 0));
        //printf("a = %s\n", retStr.c_str());

	vector<string> ret = SplitString(retStr," ");
	latOut = atof(ret[0].c_str());
	lonOut = atof(ret[1].c_str());
	heOut = 0.0;
}

void OSTN02Perl::ConvertWgs84ToGbos1936(double lat, double lon, double he,
	double &eaOut, double &noOut, double &heOut)
{

	//const char *scripts[] = {"","OSTN02latlontogrid.pl",0};
	//perl_parse(my_perl, NULL, 0, (char **)scripts, NULL);
	//PL_exit_flags |= PERL_EXIT_DESTRUCT_END;

	string cmd = "$a = OSTN02latlontogridstr(";
	cmd += ToString<double>(lat);
	cmd += ",";
	cmd += ToString<double>(lon);
	cmd += ");";

	eval_pv(cmd.c_str(), TRUE);
	string retStr = SvPV_nolen(get_sv("a", 0));
        //printf("a = %s\n", retStr.c_str());

	vector<string> ret = SplitString(retStr," ");
	eaOut = atof(ret[0].c_str());
	noOut = atof(ret[1].c_str());
	heOut = 0.0;
}

