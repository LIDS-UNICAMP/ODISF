/*****************************************************************************\
* RunODISF.c
*
* AUTHOR  : Felipe Belem
* DATE    : 2021-05-15
* LICENSE : MIT License
* EMAIL   : felipe.belem@ic.unicamp.br
\*****************************************************************************/
#include "ift.h"
#include "iftArgs.h"
#include "iftODISF.h"

void usage();
void readImgInputs
(const iftArgs *args, iftImage **img, iftImage **mask, iftImage **objsm, 
 const char **path);
void setODISFParams
(iftODISF **odisf, const iftArgs *args);
void setODISFSampl
(iftODISF **odisf, const iftArgs *args);

int main(int argc, char const *argv[])
{
  //-------------------------------------------------------------------------//
  bool has_req, has_help;
  iftArgs *args;

  args = iftCreateArgs(argc, argv);

  has_req = iftExistArg(args, "img")  && iftExistArg(args, "out");
  has_help = iftExistArg(args, "help");

  if(has_req == false || has_help == true)
  {
    usage(); 
    iftDestroyArgs(&args);
    return EXIT_FAILURE;
  }
  //-------------------------------------------------------------------------//
  const char *LABEL_PATH;
  iftImage *img, *mask, *objsm, *labels;
  iftODISF *odisf;

  readImgInputs(args, &img, &mask, &objsm, &LABEL_PATH);

  odisf = iftCreateODISF(img, mask, objsm);
  iftDestroyImage(&img);
  if(mask != NULL) iftDestroyImage(&mask);
  if(objsm != NULL) iftDestroyImage(&objsm);

  setODISFParams(&odisf, args);
  setODISFSampl(&odisf, args);
  iftDestroyArgs(&args);
  
  labels = iftRunODISF(odisf);
  iftDestroyODISF(&odisf);
  
  iftWriteImageByExt(labels, LABEL_PATH);
  iftDestroyImage(&labels);

  return EXIT_SUCCESS;
}

void usage()
{
  const int SKIP_IND = 15; // For indentation purposes
  printf("\nThe required parameters are:\n");
  printf("%-*s %s\n", SKIP_IND, "--img", 
         "Input 2D image");
  printf("%-*s %s\n", SKIP_IND, "--out", 
         "Output 2D label image");

  printf("\nThe optional parameters are:\n");
  printf("%-*s %s\n", SKIP_IND, "--no-diag-adj", 
         "Disable search scope to consider 8-adjacency.");
  printf("%-*s %s\n", SKIP_IND, "--mask", 
         "Mask image indicating the region of interest.");
  printf("%-*s %s\n", SKIP_IND, "--n0", 
         "Desired initial number of seeds. Default: 8000");
  printf("%-*s %s\n", SKIP_IND, "--nf", 
         "Desired final number of superpixels. Default: 200");
  printf("%-*s %s\n", SKIP_IND, "--sampl-op", 
         "Seed sampling algorithm. Options: "
         "grid, rnd. Default: grid");
  printf("%-*s %s\n", SKIP_IND, "--objsm", 
         "Grayscale object saliency map.");
  printf("%-*s %s\n", SKIP_IND, "--help", 
         "Prints this message");
  printf("\n");
}

void readImgInputs
(const iftArgs *args, iftImage **img, iftImage **mask, iftImage **objsm, 
 const char **path)
{
  #if IFT_DEBUG //-----------------------------------------------------------//
  assert(args != NULL);
  assert(img != NULL);
  assert(mask != NULL);
  assert(objsm != NULL);
  assert(path != NULL);
  #endif //------------------------------------------------------------------//
  const char *PATH;

  if(iftHasArgVal(args, "img") == true) 
    (*img) = iftReadImageByExt(iftGetArg(args, "img"));
  else iftError("No image path was given", "readImgInputs");

  if(iftHasArgVal(args, "out") == true)
  {
    PATH = iftGetArg(args, "out");

    if(iftIsImagePathnameValid(PATH) == true) (*path) = PATH;
    else iftError("Unknown image type", "readImgInputs");
  }
  else iftError("No output label path was given", "readImgInputs");

  if(iftExistArg(args, "mask") == true)
  {
    if(iftHasArgVal(args, "mask") == true) 
    {
      (*mask) = iftReadImageByExt(iftGetArg(args, "mask"));

      iftVerifyImageDomains(*img, *mask, "readImgInputs");
    }
    else iftError("No mask path was given", "readImgInputs");
  }
  else (*mask) = NULL;

  if(iftExistArg(args, "objsm") == true)
  {
    if(iftHasArgVal(args, "objsm") == true) 
    {
      (*objsm) = iftReadImageByExt(iftGetArg(args, "objsm"));

      iftVerifyImageDomains(*img, *objsm, "readImgInputs");
    }
    else iftError("No object saliency map path was given", "readImgInputs");
  }
  else (*objsm) = NULL;
}

void setODISFParams
(iftODISF **odisf, const iftArgs *args)
{
  #if IFT_DEBUG //-----------------------------------------------------------//
  assert(odisf != NULL);
  assert(*odisf != NULL);
  assert(args != NULL);
  #endif //------------------------------------------------------------------//
  int n0, nf;

  iftODISFUseDiagAdj(odisf, !iftExistArg(args, "no-diag-adj"));

  if(iftExistArg(args, "n0") == true)
  {
    if(iftHasArgVal(args, "n0") == true) 
    {
      n0 = atoi(iftGetArg(args, "n0"));
      iftODISFSetN0(odisf, n0);
    }
    else iftError("No initial number of seeds was given", "setODISFParams");
  }
  
  if(iftExistArg(args, "nf") == true)
  {
    if(iftHasArgVal(args, "nf") == true) 
    {
      nf = atoi(iftGetArg(args, "nf"));
      iftODISFSetNf(odisf, nf);
    }
    else iftError("No desired quantity of superpixels was given", 
                  "setODISFParams");
  }
}

void setODISFSampl
(iftODISF **odisf, const iftArgs *args)
{
  #if IFT_DEBUG //-----------------------------------------------------------//
  assert(odisf != NULL);
  assert(*odisf != NULL);
  assert(args != NULL);
  #endif //------------------------------------------------------------------//
  if(iftExistArg(args, "sampl-op") == true)
  {
    if(iftHasArgVal(args, "sampl-op") == true)
    {
      const char *OPT;

      OPT = iftGetArg(args, "sampl-op");

      if(iftCompareStrings(OPT, "grid")) 
        iftODISFSetSamplOpt(odisf, IFT_ODISF_SAMPL_GRID);
      else if(iftCompareStrings(OPT, "rnd")) 
        iftODISFSetSamplOpt(odisf, IFT_ODISF_SAMPL_RND);
      else iftError("Unknown seed sampling algorithm", "setODISFSampl");
    }
    else iftError("No sampling algorithm was given", "setODISFSampl");
  }
}