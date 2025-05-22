#ifndef HISTOMANAGER_H
#define HISTOMANAGER_H

#include <iostream>

#include "TFile.h"
#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
#include "TH2Poly.h"
#include "THashTable.h"
#include "TObject.h"
#include "TString.h"
#include "TDirectory.h"

class TProfile;
class TProfile2D;

struct HistoConfig1D{
	TString type = "";

	TString name;
	TString title;
	Int_t nbinsx;
	Double_t xmin, xmax;
	TString directory;
};

struct HistoConfig2D{
	TString type = "";

	TString name;
	TString title;
	Int_t nbinsx;
	Double_t xmin, xmax;
	Int_t nbinsy;
	Double_t ymin, ymax;
	TString directory;
};

struct HistoConfig2DPoly{
	TString type = "";

	TString name;
	TString title;
	TString directory;
};

struct HistoConfig3D{
	TString type = "";

	TString name;
	TString title;
	Int_t nbinsx;
	Double_t xmin, xmax;
	Int_t nbinsy;
	Double_t ymin, ymax;
	Int_t nbinsz;
	Double_t zmin, zmax;
	TString directory;
};

class HistoManager {

public:
	HistoManager(TFile *outputfile = nullptr);
	virtual ~HistoManager();

	bool loadHistoConfig(const TString& configFilePath);

	void addHisto1D(const TString& name, const TString& title, Int_t nbinsX, Double_t xmin, Double_t xmax, const TString& type, const TString& directory);
	void addHisto1D(const HistoConfig1D& config);

	void addHisto2D(const TString& name, const TString& title, Int_t nbinsX, Double_t xmin, Double_t xmax, Int_t nbinsY, Double_t ymin, Double_t ymax, const TString& type, const TString& directory);
	void addHisto2D(const HistoConfig2D& config);

	void addHisto2DPoly(const TString& name, const TString& title, const TString& directory);
	void addHisto2DPoly(const HistoConfig2DPoly& config);

	void addHisto3D(const TString& name, const TString& title, Int_t nbinsX, Double_t xmin, Double_t xmax, Int_t nbinsY, Double_t ymin, Double_t ymax, Int_t nbinsZ, Double_t zmin, Double_t zmax, const TString& type, const TString& directory);
	void addHisto3D(const HistoConfig3D& config);

	TH1* getHisto1D(const TString& name) const;
	TH2* getHisto2D(const TString& name) const;
	TH3* getHisto3D(const TString& name) const;
	TProfile* getProfile1D(const TString& name) const;
	TProfile2D* getProfile2D(const TString& name) const;
	TH2Poly* getHisto2DPoly(const TString& name) const;

	void WriteAll(bool writeFileToDiskAutomatically=true);
	void Write(const TString& name);
	void Write(const TString& name, TDirectory *tdir);

protected:
	TFile *m_outputFile;
	THashTable m_h1DTable;
	THashTable m_h2DTable;
	THashTable m_h3DTable;
	THashTable m_profile1DTable;
	THashTable m_profile2DTable;
	THashTable m_h2DPolyTable;

private:
	TH1* createHisto1D(const HistoConfig1D& config);
	TH2* createHisto2D(const HistoConfig2D& config);
	TH3* createHisto3D(const HistoConfig3D& config);
	TDirectory* getOrCreateDirectory(const TString& path);
};

#endif