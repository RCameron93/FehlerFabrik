#include "plugin.hpp"

Plugin *pluginInstance;

void init(Plugin *p)
{
	pluginInstance = p;

	// Add modules here
	// p->addModel(modelMyModule);
	p->addModel(modelPSIOP);
	p->addModel(modelPlanck);
	p->addModel(modelLuigi);
	p->addModel(modelAspect);
	p->addModel(modelMonte);
	p->addModel(modelArpanet);
	p->addModel(modelSigma);
	p->addModel(modelFax);
	p->addModel(modelRasoir);
	p->addModel(modelChi);
	p->addModel(modelNova);
	p->addModel(modelLilt);
	p->addModel(modelBotzinger);

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}
