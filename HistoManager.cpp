#include "HistoManager.h"
#include "TH1F.h"
#include "TH1D.h"
#include "TH2F.h"
#include "TH2D.h"
#include "TProfile.h"
#include "TProfile2D.h"
#include "TROOT.h"
#include "TDirectory.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <string>

HistoManager::HistoManager(TFile* outputfile) : m_outputFile(outputfile){
	if(m_outputFile){
		m_outputFile->cd();
	}
}

HistoManager::~HistoManager(){

	TIter next1D(m_h1DTable.MakeIterator());
	TObject *obj1D;
	while((obj1D = next1D())){
		delete obj1D;
	}
	m_h1DTable.Clear();

	TIter next2D(m_h2DTable.MakeIterator());
	TObject *obj2D;
	while((obj2D = next2D())){
		delete obj2D;
	}
	m_h2DTable.Clear();

	TIter nextProfile1D(m_profile1DTable.MakeIterator());
	TObject *objp1D;
	while((objp1D = nextProfile1D())){
		delete objp1D;
	}
	m_profile1DTable.Clear();

	TIter nextProfile2D(m_profile2DTable.MakeIterator());
	TObject *objp2D;
	while((objp2D = nextProfile2D())){
		delete objp2D;
	}
	m_profile2DTable.Clear();

}

bool HistoManager::loadHistoConfig(const TString& configFilePath){
	ifstream configFile(configFilePath.Data());
	if(!configFile.is_open()){
		cerr << "Error: Could not open config file: " << configFilePath << endl;
		return false;
	}

	string line;
	while(getline(configFile,line)){
		if(line.empty() || line[0] == '#'){
			continue;
		}

		istringstream iss(line);
		string type;
		if(!(iss >> type)){
			cerr << "Error: Could not read histogram type from line: " << line << endl;
			continue;
		}

		if(type == "TH1F" || type == "TH1D" || type == "TProfile"){
			HistoConfig1D config;
			config.type = type;
			if(iss >> config.name >> config.title >> config.nbinsx >> config.xmin >> config.xmax){
				addHisto1D(config);
			} else {
				cerr << "Error: Invalid 1D Histogram configuration: " << line << endl;
			}
		} else if(type == "TH2F" || type == "TH2D" || type == "TProfile2D"){
			HistoConfig2D config;
			config.type = type;
			if(iss >> config.name >> config.title >> config.nbinsx >> config.xmin >> config.xmax >> config.nbinsy >> config.ymin >> config.ymax){
				addHisto2D(config);
			} else {
				cerr << "Error: Invalid 2D Histogram configuration: " << line << endl;
			}
		} else {
			cerr << "Error: Unknown histogram type: " << type << " in line: " << line << endl;
		}
	}

	configFile.close();
	return true;
}

void HistoManager::addHisto1D(const TString& name, const TString& title, Int_t nbinsx, Double_t xmin, Double_t xmax, const TString& type){
	HistoConfig1D config;
	config.name = name;
	config.title = title;
	config.nbinsx = nbinsx;
	config.xmin = xmin;
	config.xmax = xmax;
	config.type = type;
	addHisto1D(config);
}

void HistoManager::addHisto1D(const HistoConfig1D& config){
	TH1* h = createHisto1D(config);
	if(h){
		m_h1DTable.Add(h);
	}
}

void HistoManager::addHisto2D(const TString& name, const TString& title, Int_t nbinsx, Double_t xmin, Double_t xmax, Int_t nbinsy, Double_t ymin, Double_t ymax, const TString& type){
	HistoConfig2D config;
	config.name = name;
	config.title = title;
	config.nbinsx = nbinsx;
	config.xmin = xmin;
	config.xmax = xmax;
	config.nbinsy = nbinsy;
	config.ymin = ymin;
	config.ymax = ymax;
	config.type = type;
	addHisto2D(config);
}

void HistoManager::addHisto2D(const HistoConfig2D& config){
	TH2* h = createHisto2D(config);
	if(h){
		m_h2DTable.Add(h);
	}
}

TH1* HistoManager::getHisto1D(const TString& name) const{
	return dynamic_cast<TH1*>(m_h1DTable.FindObject(name.Data()));
}

TH2* HistoManager::getHisto2D(const TString& name) const{
	return dynamic_cast<TH2*>(m_h2DTable.FindObject(name.Data()));
}

TProfile* HistoManager::getProfile1D(const TString& name) const {
	return dynamic_cast<TProfile*>(m_profile1DTable.FindObject(name.Data()));
}

TProfile2D* HistoManager::getProfile2D(const TString& name) const {
	return dynamic_cast<TProfile2D*>(m_profile2DTable.FindObject(name.Data()));
}

void HistoManager::WriteAll(bool writeFileToDiskAutomatically){
	if(m_outputFile){
		m_outputFile->cd();

		TIter next1D(m_h1DTable.MakeIterator());
		TObject *obj1D;
		while((obj1D = next1D())){
			obj1D->Write();
		}

		TIter next2D(m_h2DTable.MakeIterator());
		TObject *obj2D;
		while((obj2D = next2D())){
			obj2D->Write();
		}

		TIter nextP1D(m_profile1DTable.MakeIterator());
		TObject *objP1D;
		while((objP1D = nextP1D())){
			objP1D->Write();
		}

		TIter nextP2D(m_profile2DTable.MakeIterator());
		TObject *objP2D;
		while((objP2D = nextP2D())){
			objP2D->Write();
		}

		if(writeFileToDiskAutomatically) m_outputFile->Write();
	} else {
		cerr << "Warning: Output file not set. Histograms will not be written to disk." << endl;
	}
}

void HistoManager::Write(const TString& name){
	if(m_outputFile){
		m_outputFile->cd();
		Write(name,m_outputFile);
	} else {
		cerr << "Warning: Output file not set. Histogram " << name << " will not be written to disk." << endl;
	}
}

void HistoManager::Write(const TString& name, TDirectory *tdir){
	TObject *obj = nullptr;

	obj = m_h1DTable.FindObject(name.Data());
	if(obj){
		tdir->cd();
		obj->Write();
		return;
	}

	obj = m_h2DTable.FindObject(name.Data());
	if(obj){
		tdir->cd();
		obj->Write();
		return;
	}

	obj = m_profile1DTable.FindObject(name.Data());
	if(obj){
		tdir->cd();
		obj->Write();
		return;
	}

	obj = m_profile2DTable.FindObject(name.Data());
	if(obj){
		tdir->cd();
		obj->Write();
		return;
	}

	cerr << "Error: Histogram " << name << " not found." << endl;
}

TH1* HistoManager::createHisto1D(const HistoConfig1D& config){
	TH1* histo = nullptr;
	if(config.type == "TH1F"){
		histo = new TH1F(config.name, config.title, config.nbinsx, config.xmin, config.xmax);
	} else if(config.type == "TH1D"){
		histo = new TH1D(config.name, config.title, config.nbinsx, config.xmin, config.xmax);
	} else if(config.type == "TProfile"){
		histo = new TProfile(config.name, config.title, config.nbinsx, config.xmin, config.xmax);
	} else {
		cerr << "Error: Unknown 1D histogram type: " << config.type.Data() << endl;
		cerr << "Did you forget to add functionality for " << config.type.Data() << " in HistoManager class?" << endl;
	}
	return histo;
}

TH2* HistoManager::createHisto2D(const HistoConfig2D& config){
	TH2* histo = nullptr;
	if(config.type == "TH2F"){
		histo = new TH2F(config.name, config.title, config.nbinsx, config.xmin, config.xmax, config.nbinsy, config.ymin, config.ymax);
	} else if(config.type == "TH2D"){
		histo = new TH2D(config.name, config.title, config.nbinsx, config.xmin, config.xmax, config.nbinsy, config.ymin, config.ymax);
	} else if(config.type == "TProfile2D"){
		histo = new TProfile2D(config.name, config.title, config.nbinsx, config.xmin, config.xmax, config.nbinsy, config.ymin, config.ymax);
	} else {
		cerr << "Error: Unknown 2D histogram type: " << config.type.Data() << endl;
		cerr << "Did you forget to add functionality for " << config.type.Data() << " in HistoManager class?" << endl;
	}
	return histo;
}
