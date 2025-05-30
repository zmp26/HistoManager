/*
	Usage:
	.L HistoManager.cpp+				(this loads as a shared library)
	HistoManager *histomanager = new HistoManager(ExistingTFilePointer);
	histomanager->
*/

#include "HistoManager.h"
#include "TH1F.h"
#include "TH1D.h"
#include "TH1I.h"
#include "TH2F.h"
#include "TH2D.h"
#include "TH2I.h"
#include "TH3F.h"
#include "TH3D.h"
#include "TH3I.h"
#include "TProfile.h"
#include "TProfile2D.h"
#include "TROOT.h"
#include "TDirectory.h"
#include "TObjString.h"
#include "TObject.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

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
		string directory;
		string type;
		if(!(iss >> directory >> type)){
			cerr << "Error: Could not read histogram type from line: " << line << endl;
			continue;
		}

		if(type == "TH1F" || type == "TH1D" || type == "TH1I" || type == "TProfile"){
			HistoConfig1D config;
			config.directory = directory;
			config.type = type;
			if(iss >> config.name >> config.title >> config.nbinsx >> config.xmin >> config.xmax){
				addHisto1D(config);
			} else {
				cerr << "Error: Invalid 1D Histogram configuration: " << line << endl;
			}
		} else if(type == "TH2F" || type == "TH2D" || type == "TH1I" || type == "TProfile2D"){
			HistoConfig2D config;
			config.directory = directory;
			config.type = type;
			if(iss >> config.name >> config.title >> config.nbinsx >> config.xmin >> config.xmax >> config.nbinsy >> config.ymin >> config.ymax){
				addHisto2D(config);
			} else {
				cerr << "Error: Invalid 2D Histogram configuration: " << line << endl;
			}
		} else if(type == "TH3F" || type == "TH3D" || type == "TH3I"){
			HistoConfig3D config;
			config.directory = directory;
			config.type = type;
			if(iss >> config.name >> config.title >> config.nbinsx >> config.xmin >> config.xmax >> config.nbinsy >> config.ymin >> config.ymax >> config.nbinsz >> config.zmin >> config.zmax){
				addHisto3D(config);
			} else {
				cerr << "Error: Invalid 3D Histogram configuration: " << line << endl;
			}
		} else if(type == "TH2Poly"){
			HistoConfig2DPoly config;
			config.directory = directory;
			config.type = type;
			if(iss >> config.name >> config.title) {
				addHisto2DPoly(config);
				//can parse polygons here in the future
			} else {
				cerr << "Error: Invalid TH2Poly Histogram configuration: " << line << endl;
			}
		} else {
			cerr << "Error: Unknown histogram type: " << type << " in line: " << line << endl;
		}
	}

	configFile.close();
	return true;
}

void HistoManager::addHisto1D(const TString& name, const TString& title, Int_t nbinsx, Double_t xmin, Double_t xmax, const TString& type, const TString& directory){
	HistoConfig1D config;
	config.name = name;
	config.title = title;
	config.nbinsx = nbinsx;
	config.xmin = xmin;
	config.xmax = xmax;
	config.type = type;
	config.directory = directory;
	addHisto1D(config);
}

void HistoManager::addHisto1D(const HistoConfig1D& config){
	if(getHisto1D(config.name)){
		cerr << "Warning: Histogram " << config.name << " already exists. Skipping." << endl;
		return;
	}
	TH1* h = createHisto1D(config);
	if(h){
		TDirectory* dir = getOrCreateDirectory(config.directory);
		dir->cd();
		h->SetDirectory(dir);
		if(config.type == "TProfile"){
			m_profile1DTable.Add(h);
		} else {
			m_h1DTable.Add(h);
		}
		if(m_outputFile) m_outputFile->cd();
	}
}

void HistoManager::addHisto2D(const TString& name, const TString& title, Int_t nbinsx, Double_t xmin, Double_t xmax, Int_t nbinsy, Double_t ymin, Double_t ymax, const TString& type, const TString& directory){
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
	config.directory = directory;
	addHisto2D(config);
}

void HistoManager::addHisto2D(const HistoConfig2D& config){
	if(getHisto2D(config.name)){
		cerr << "Warning: Histogram " << config.name << " already exists. Skipping." << endl;
		return;
	}
	TH2* h = createHisto2D(config);
	if(h){
		TDirectory* dir = getOrCreateDirectory(config.directory);
		dir->cd();
		h->SetDirectory(dir);
		if(config.type == "TProfile2D"){
			m_profile2DTable.Add(h);
		} else {
			m_h2DTable.Add(h);
		}
		if(m_outputFile) m_outputFile->cd();
	}
}

void HistoManager::addHisto2DPoly(const TString& name, const TString& title, const TString& directory){
	HistoConfig2DPoly config;
	config.name = name;
	config.title = title;
	config.type = "TH2Poly";
	config.directory = directory;
	addHisto2DPoly(config);
}

void HistoManager::addHisto2DPoly(const HistoConfig2DPoly& config){
	if(getHisto2DPoly(config.name)){
		cerr << "Warning: TH2Poly " << config.name << " already exists. Skipping." << endl;
		return;
	}

	TH2Poly* h = new TH2Poly(config.name, config.title, 0, 0, 0, 0);
	TDirectory* dir = getOrCreateDirectory(config.directory);
	dir->cd();
	h->SetDirectory(dir);
	m_h2DPolyTable.Add(h);
	if(m_outputFile) m_outputFile->cd();
}

void HistoManager::addHisto3D(const TString& name, const TString& title, Int_t nbinsx, Double_t xmin, Double_t xmax, Int_t nbinsy, Double_t ymin, Double_t ymax, Int_t nbinsz, Double_t zmin, Double_t zmax, const TString& type, const TString& directory){
	HistoConfig3D config;
	config.name = name;
	config.title = title;
	config.nbinsx = nbinsx;
	config.xmin = xmin;
	config.xmax = xmax;
	config.nbinsy = nbinsy;
	config.ymin = ymin;
	config.ymax = ymax;
	config.nbinsz = nbinsz;
	config.zmin = zmin;
	config.zmax = zmax;
	config.type = type;
	config.directory = directory;
	addHisto3D(config);
}

void HistoManager::addHisto3D(const HistoConfig3D& config){
	if(getHisto3D(config.name)){
		cerr << "Warning: Histogram " << config.name << " already exists. Skipping." << endl;
		return;
	}
	TH3* h = createHisto3D(config);
	if(h){
		TDirectory* dir = getOrCreateDirectory(config.directory);
		dir->cd();
		h->SetDirectory(dir);
		m_h3DTable.Add(h);
		if(m_outputFile) m_outputFile->cd();
	}
}



TH1* HistoManager::getHisto1D(const TString& name) const{
	return dynamic_cast<TH1*>(m_h1DTable.FindObject(name.Data()));
}

TH2* HistoManager::getHisto2D(const TString& name) const{
	return dynamic_cast<TH2*>(m_h2DTable.FindObject(name.Data()));
}

TH3* HistoManager::getHisto3D(const TString& name) const{
	return dynamic_cast<TH3*>(m_h3DTable.FindObject(name.Data()));
}

TProfile* HistoManager::getProfile1D(const TString& name) const {
	return dynamic_cast<TProfile*>(m_profile1DTable.FindObject(name.Data()));
}

TProfile2D* HistoManager::getProfile2D(const TString& name) const {
	return dynamic_cast<TProfile2D*>(m_profile2DTable.FindObject(name.Data()));
}

TH2Poly* HistoManager::getHisto2DPoly(const TString& name) const {
	return dynamic_cast<TH2Poly*>(m_h2DPolyTable.FindObject(name.Data()));
}

// void HistoManager::WriteAll(bool writeFileToDiskAutomatically){
// 	if(m_outputFile){
// 		m_outputFile->cd();

// 		std::vector<TObject*> sorted1D;

// 		TIter next1D(m_h1DTable.MakeIterator());
// 		TObject *obj1D;
// 		while((obj1D = next1D())){
// 			TH1* h = dynamic_cast<TH1*>(obj1D);
// 			if(h){
// 				TDirectory* dir = h->GetDirectory();
// 				if(!dir) dir = m_outputFile;
// 				dir->cd();
// 				h->Write();
// 			}
// 		}

// 		TIter next2D(m_h2DTable.MakeIterator());
// 		TObject *obj2D;
// 		while((obj2D = next2D())){
// 			TH2* h = dynamic_cast<TH2*>(obj2D);
// 			if(h){
// 				TDirectory* dir = h->GetDirectory();
// 				if(!dir) dir = m_outputFile;
// 				dir->cd();
// 				h->Write();
// 			}
// 		}

// 		TIter nextP1D(m_profile1DTable.MakeIterator());
// 		TObject *objP1D;
// 		while((objP1D = nextP1D())){
// 			TProfile* p = dynamic_cast<TProfile*>(objP1D);
// 			if(p){
// 				TDirectory* dir = p->GetDirectory();
// 				if(!dir) dir = m_outputFile;
// 				dir->cd();
// 				p->Write();
// 			}
// 		}

// 		TIter nextP2D(m_profile2DTable.MakeIterator());
// 		TObject *objP2D;
// 		while((objP2D = nextP2D())){
// 			TProfile2D* p = dynamic_cast<TProfile2D*>(objP2D);
// 			if(p){
// 				TDirectory* dir = p->GetDirectory();
// 				if(!dir) dir = m_outputFile;
// 				dir->cd();
// 				p->Write();
// 			}
// 		}

// 		if(writeFileToDiskAutomatically) m_outputFile->Write();
// 	} else {
// 		cerr << "Warning: Output file not set. Histograms will not be written to disk." << endl;
// 	}
// }

void HistoManager::WriteAll(bool writeFileToDiskAutomatically){
	if(!m_outputFile){
		cerr << "Warning: output file not set. Histograms will not be written to disk." << endl;
		return;
	}

	std::map<TDirectory*, std::vector<TObject*>> dirMap;

	//lambda to gather histograms from any TList into the map
	auto collectHistos = [&](THashTable& table){
		TIter it(table.MakeIterator());
		TObject* obj;
		while((obj = it())){
			TDirectory* dir = nullptr;

			//determine the directroy from the actual histogram type
			if(auto h1 = dynamic_cast<TH1*>(obj)){
				dir = h1->GetDirectory();
			} else if(auto h2 = dynamic_cast<TH2*>(obj)){
				dir = h2->GetDirectory();
			} else if(auto h3 = dynamic_cast<TH3*>(obj)){
				dir = h3->GetDirectory();
			} else if(auto p1 = dynamic_cast<TProfile*>(obj)){
				dir = p1->GetDirectory();
			} else if(auto p2 = dynamic_cast<TProfile2D*>(obj)){
				dir = p2->GetDirectory();
			} else if(auto h2poly = dynamic_cast<TH2Poly*>(obj)){
				dir = h2poly->GetDirectory();
			}

			if(!dir) dir = m_outputFile;//default to root file
			dirMap[dir].push_back(obj);
		}
	};

	//collect all histograms from all types
	collectHistos(m_h1DTable);
	collectHistos(m_h2DTable);
	collectHistos(m_h3DTable);
	collectHistos(m_profile1DTable);
	collectHistos(m_profile2DTable);
	collectHistos(m_h2DPolyTable);

	//sort and write histograms in each directory
	for(auto& [dir,histos] : dirMap){
		std::sort(histos.begin(),histos.end(),[](TObject* a, TObject* b){
			return (TString(a->GetName()) < TString(b->GetName()));
		});
		dir->cd();
		for(auto* h : histos){
			h->Write("",TObject::kOverwrite);
		}
	}

	if(writeFileToDiskAutomatically){ m_outputFile->Write("",TObject::kOverwrite); } else { cout << "Histos written to file in memory but file not written to disk yet!" << endl; }
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
		obj->Write("",TObject::kOverwrite);
		return;
	}

	obj = m_h2DTable.FindObject(name.Data());
	if(obj){
		tdir->cd();
		obj->Write("",TObject::kOverwrite);
		return;
	}

	obj = m_h3DTable.FindObject(name.Data());
	if(obj){
		tdir->cd();
		obj->Write("",TObject::kOverwrite);
		return;
	}

	obj = m_profile1DTable.FindObject(name.Data());
	if(obj){
		tdir->cd();
		obj->Write("",TObject::kOverwrite);
		return;
	}

	obj = m_profile2DTable.FindObject(name.Data());
	if(obj){
		tdir->cd();
		obj->Write("",TObject::kOverwrite);
		return;
	}

	obj = m_h2DPolyTable.FindObject(name.Data());
	if(obj){
		tdir->cd();
		obj->Write("",TObject::kOverwrite);
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
	} else if(config.type == "TH1I") {
		histo = new TH1I(config.name, config.title, config.nbinsx, config.xmin, config.xmax);
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
	} else if(config.type == "TH2I"){
		histo = new TH2I(config.name, config.title, config.nbinsx, config.xmin, config.xmax, config.nbinsy, config.ymin, config.ymax);
	} else if(config.type == "TProfile2D"){
		histo = new TProfile2D(config.name, config.title, config.nbinsx, config.xmin, config.xmax, config.nbinsy, config.ymin, config.ymax);
	} else {
		cerr << "Error: Unknown 2D histogram type: " << config.type.Data() << endl;
		cerr << "Did you forget to add functionality for " << config.type.Data() << " in HistoManager class?" << endl;
	}
	return histo;
}

TH3* HistoManager::createHisto3D(const HistoConfig3D& config){
	TH3* histo = nullptr;
	if(config.type == "TH3F"){
		histo = new TH3F(config.name, config.title, config.nbinsx, config.xmin, config.xmax, config.nbinsy, config.ymin, config.ymax, config.nbinsz, config.zmin, config.zmax);
	} else if(config.type == "TH3D"){
		histo = new TH3D(config.name, config.title, config.nbinsx, config.xmin, config.xmax, config.nbinsy, config.ymin, config.ymax, config.nbinsz, config.zmin, config.zmax);
	} else if(config.type == "TH3I"){
		histo = new TH3I(config.name, config.title, config.nbinsx, config.xmin, config.xmax, config.nbinsy, config.ymin, config.ymax, config.nbinsz, config.zmin, config.zmax);
	} else {
		cerr << "Error: Unknown 3D histogram type: " << config.type.Data() << endl;
		cerr << "Did you forget to add functionality for " << config.type.Data() << " in HistoManager class?" << endl;
	}
	return histo;
}

TDirectory* HistoManager::getOrCreateDirectory(const TString& path){
	if(!m_outputFile){
		cerr << "Warning: Output file not set. Cannot create directory." << endl;
		return gDirectory;
	}

	if(path.IsNull() || path.IsWhitespace()){
		return m_outputFile;
	}

	TDirectory* currentDir = m_outputFile;
	TObjArray* parts = path.Tokenize("/");

	TIter next(parts);

	TObjString* obj;
	while((obj = (TObjString*)next())){
		TString part = obj->GetString();
		TDirectory *subdir = (TDirectory*) currentDir->Get(part);
		if(!subdir){
			subdir = currentDir->mkdir(part);
			if(!subdir){
				cerr << "Error creating directory: " << path << ". Defaulting to current directory." << endl;
				delete parts;
				return gDirectory;
			}
		}
		currentDir = subdir;
	}

	delete parts;
	return currentDir;
}