#include "hdf5_utils.h"

#include <stdlib.h>
#include "k.h"
#include "hdf5.h"
#include "kdb_utils.h"

EXP K hdf5createFile(K fname){
  if(!kdbCheckType("[Cs]", fname))
    return KNL;
  char *filename = kdbGetString(fname);
  H5Fcreate(filename, H5F_ACC_EXCL, H5P_DEFAULT, H5P_DEFAULT);
  // clean up
  free(filename);
  return KNL;
}

EXP K hdf5createGroup(K fname, K gname){
  if(!kdbCheckType("[Cs][Cs]", fname, gname))
    return KNL;
  hid_t file, group;
  hid_t gcpl; // group creation property list
  char *filename   = kdbGetString(fname);
  htri_t filechk = H5Fis_hdf5(filename);
  if(filechk < 0)
    H5Fcreate(filename, H5F_ACC_EXCL, H5P_DEFAULT, H5P_DEFAULT);
  if(filechk == 0){
    free(filename);
    return krr((S)"file is not hdf5");
  }
  file = H5Fopen(filename, H5F_ACC_RDWR, H5P_DEFAULT);
  char *groupnames = kdbGetString(gname);
  gcpl = H5Pcreate(H5P_LINK_CREATE);
  H5Pset_create_intermediate_group(gcpl, 1); // create intermediate groups
  group = H5Gcreate(file, groupnames, gcpl, H5P_DEFAULT, H5P_DEFAULT);
  if(group < 0)
    krr((S)"Group could not be created");
  // clean up
  free(filename);
  free(groupnames);
  H5Gclose(group);
  H5Pclose(gcpl);
  H5Fclose(file);
  return KNL;
}

EXP K hdf5createDataset(K fname, K dname, K kdims, K ktype){
  if(!kdbCheckType("[Cs][Cs]Ic", fname, dname, kdims, ktype))
    return KNL;
  hid_t file;
  ktype_t dtype;
  char *filename = kdbGetString(fname);
  char *dataname = kdbGetString(dname);
  if(H5Fis_hdf5(filename) <= 0) // create file if it does not exist
    H5Fcreate(filename, H5F_ACC_EXCL, H5P_DEFAULT, H5P_DEFAULT);
  file = H5Fopen(filename, H5F_ACC_RDWR, H5P_DEFAULT);
  dtype = getKType(ktype->g);
  if(dtype == NUMERIC)
    createNumericDataset(file, dataname, kdims, ktype);
  else if(dtype == STRING)
    createStringDataset(file, dataname, kdims);
  // clean up
  free(filename);
  free(dataname);
  H5Fclose(file);
  return KNL;
}

EXP K hdf5createAttr(K fname, K dname, K aname, K kdims, K ktype){
  if(!kdbCheckType("[Cs][Cs][Cs]Ic", fname, dname, aname, kdims, ktype))
    return KNL;
  hid_t file, data;
  ktype_t  dtype;
  char *filename = kdbGetString(fname);
  file = H5Fopen(filename, H5F_ACC_RDWR, H5P_DEFAULT);
  if(file < 0){
    free(filename);
    return krr((S)"file does not exist");
  }
  char *dataname = kdbGetString(dname);
  data = openGroupData(file, dataname);
  if(data < 0){
    free(filename);
    free(dataname);
    H5Fclose(file);
    return krr((S)"group/dataset does not exist");
  }
  char *attrname = kdbGetString(aname);
  if(H5Aexists(data, attrname) > 0){
    closeGroupData(file, dataname, data);
    H5Fclose(file);
    free(filename);
    free(dataname);
    free(attrname);
    return krr((S)"attribute already exists for group/dataset");
  }
  dtype = getKType(ktype->g);
  if(dtype == NUMERIC)
    createNumericAttribute(data, attrname, kdims, ktype);
  else if(dtype == STRING)
    createStringAttribute(data, attrname, kdims);
  // clean up
  closeGroupData(file,dataname,data);
  H5Fclose(file);
  free(filename);
  free(dataname);
  free(attrname);
  return KNL;
}
