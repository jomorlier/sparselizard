#include "universe.h"


mesh* universe::mymesh = NULL;

double universe::currenttimestep = 0;

double universe::fundamentalfrequency = -1;
double universe::getfundamentalfrequency(void)
{
    if (fundamentalfrequency > 0)
        return fundamentalfrequency;
    else
    {
        std::cout << "Error in 'universe' object: the fundamental frequency cannot be negative or 0 (make sure it was set)" << std::endl;
        abort();
    }
}

bool universe::isreuseallowed = false;

void universe::allowreuse(void)
{
    isreuseallowed = true;
}

void universe::forbidreuse(void)
{
    isreuseallowed = false;
    
    computedformfuncs = {};
    
    computedjacobian = NULL;
    
    oppointers = {};
    oppointersfft = {};
    opcomputed = {};
    opcomputedfft = {};
}

std::vector<std::pair< std::string, std::vector<std::vector< std::pair<int,hierarchicalformfunctioncontainer> >> >> universe::computedformfuncs = {};
hierarchicalformfunctioncontainer universe::interpolateformfunction(std::string fftypename, int elementtypenumber, int interpolorder, std::vector<double> evaluationcoordinates)
{
    // Find the type name in the container:
    int typenameindex = -1;
    for (int i = 0; i < computedformfuncs.size(); i++)
    {
        if (computedformfuncs[i].first == fftypename)
        {
            typenameindex = i;
            break;
        }
    }
    
    // In case the form function is available and we are allowed to reuse it:
    if (isreuseallowed && typenameindex != -1 && computedformfuncs[typenameindex].second.size() > elementtypenumber && computedformfuncs[typenameindex].second[elementtypenumber].size() > 0 && computedformfuncs[typenameindex].second[elementtypenumber][0].first >= interpolorder)
        return computedformfuncs[typenameindex].second[elementtypenumber][0].second;
    
    // Otherwise it must be computed:
    std::shared_ptr<hierarchicalformfunction> myformfunction = selector::select(elementtypenumber, fftypename);
    hierarchicalformfunctioncontainer val = myformfunction->evalat(interpolorder, evaluationcoordinates);
    
    // Store it for reuse if allowed:
    if (isreuseallowed)
    {
        if (typenameindex == -1)
            computedformfuncs.push_back( std::make_pair(fftypename, std::vector<std::vector< std::pair<int,hierarchicalformfunctioncontainer> >>(8,std::vector< std::pair<int,hierarchicalformfunctioncontainer> >(0))) );
        typenameindex = computedformfuncs.size() - 1;
        computedformfuncs[typenameindex].second[elementtypenumber] = {std::make_pair(interpolorder, val)};
    }
    
    return val;
}

shared_ptr<jacobian> universe::computedjacobian = NULL;


std::vector<shared_ptr<operation>> universe::oppointers = {};
std::vector<shared_ptr<operation>> universe::oppointersfft = {};
std::vector< std::vector<std::vector<densematrix>> > universe::opcomputed = {};
std::vector< densematrix > universe::opcomputedfft = {};

int universe::getindexofprecomputedvalue(shared_ptr<operation> op)
{
    for (int i = 0; i < oppointers.size(); i++)
    {
        if (oppointers[i].get() == op.get())
            return i;
    }
    return -1;
}

int universe::getindexofprecomputedvaluefft(shared_ptr<operation> op)
{
    for (int i = 0; i < oppointersfft.size(); i++)
    {
        if (oppointersfft[i].get() == op.get())
            return i;
    }
    return -1;
}

std::vector<std::vector<densematrix>> universe::getprecomputed(int index)
{
    std::vector<std::vector<densematrix>> output = opcomputed[index];
    for (int h = 0; h < output.size(); h++)
    {
        if (output[h].size() == 1)
            output[h][0] = output[h][0].copy();
    }
    return output;
}

densematrix universe::getprecomputedfft(int index)
{
    return (opcomputedfft[index]).copy();
}

void universe::setprecomputed(shared_ptr<operation> op, std::vector<std::vector<densematrix>> val)
{
    oppointers.push_back(op);
    opcomputed.push_back(val);
    for (int h = 0; h < val.size(); h++)
    {
        if (val[h].size() == 1)
            opcomputed[opcomputed.size()-1][h][0] = val[h][0].copy();
    }
}

void universe::setprecomputedfft(shared_ptr<operation> op, densematrix val)
{
    oppointersfft.push_back(op);
    opcomputedfft.push_back(val.copy());
}
        
        
        






        
        