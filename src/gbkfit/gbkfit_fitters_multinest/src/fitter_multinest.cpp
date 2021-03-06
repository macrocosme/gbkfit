
#include "gbkfit/fitters/multinest/fitter_multinest.hpp"

#include "gbkfit/model.hpp"
#include "gbkfit/nddataset.hpp"
#include "gbkfit/ndarray_host.hpp"
#include "gbkfit/parameters_fit_info.hpp"
#include "gbkfit/utility.hpp"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <multinest.h>

namespace gbkfit {
namespace fitters {
namespace multinest {

struct multinest_user_data
{
    gbkfit::model* model;
    std::vector<std::string> param_name;
    std::vector<bool> param_fixed;
    std::vector<float> param_value;
    std::vector<float> param_min;
    std::vector<float> param_max;
    std::vector<float> param_mean;
    std::vector<float> param_stddev;
    std::vector<float> param_best;
    std::vector<float> param_map;
    std::vector<std::string> dataset_names;
    std::map<std::string,ndarray_host*> data_map_dat;
    std::map<std::string,ndarray_host*> data_map_msk;
    std::map<std::string,ndarray_host*> data_map_err;
    std::map<std::string,ndarray_host*> data_map_mdl;
}; // struct multinest_user_data

void multinest_callback_likelihood(double* cube, int& ndim, int& npars, double& lnew, void* context)
{
    (void)ndim;
    (void)npars;

    //
    // Get a convenience pointer to the user data.
    //

    multinest_user_data* udata = reinterpret_cast<multinest_user_data*>(context);

    //
    //  Update model parameter values.
    //

    int free_params_counter = 0;
    for(std::size_t i = 0; i < udata->param_name.size(); ++i)
    {
        if(!udata->param_fixed[i])
        {
            udata->param_value[i] = udata->param_min[i] + (udata->param_max[i] - udata->param_min[i]) * cube[free_params_counter];
            cube[free_params_counter] = udata->param_value[i];
            free_params_counter++;
        }
    }

    //
    // Build parameter map with the current set of parameter values.
    //

    const std::map<std::string,float> param_map = gbkfit::vectors_to_map(udata->param_name, udata->param_value);

#if 0
    // Provide debug information about the current set of parameters.
    std::cout << gbkfit::to_string(param_map) << std::endl;
#endif

    //
    // Evaluate model with the current set of parameter values.
    //

    std::map<std::string,ndarray*> model_data = udata->model->evaluate(param_map);

    //
    // Calculate likelihood between the model and the (available) data.
    //

    float chi2 = 0;
    for(auto& dataset_name : udata->dataset_names)
    {
        // Get number of measurements in the current data.
        int data_size = udata->data_map_dat[dataset_name]->get_shape().get_dim_length_product();

        // Create convenience pointers.
        float* data_dat = udata->data_map_dat[dataset_name]->get_host_ptr();
        float* data_msk = udata->data_map_msk[dataset_name]->get_host_ptr();
        float* data_err = udata->data_map_err[dataset_name]->get_host_ptr();
        float* data_mdl = udata->data_map_mdl[dataset_name]->get_host_ptr();

        // Copy model data on the host.
        model_data[dataset_name]->read_data(data_mdl);

        // Calculate chi squared.
        for(int i = 0; i < data_size; ++i) {
            float residual = data_msk[i] > 0 ? (data_dat[i]-data_mdl[i])/data_err[i] : 0;
            chi2 += residual*residual;
        }
    }

    // Save likelihood! Woohoo!
    lnew = -0.5*chi2;
}

void multinest_callback_dumper(int& nsamples, int& nlive, int& npar, double** physlive, double** posterior, double** paramconstr, double& maxloglike, double& logz, double& inslogz, double& logzerr, void* context)
{
    (void)nsamples;
    (void)nlive;
    (void)physlive;
    (void)posterior;
    (void)maxloglike;
    (void)logz;
    (void)inslogz;
    (void)logzerr;

    multinest_user_data* udata = reinterpret_cast<multinest_user_data*>(context);

    const std::map<std::string,float> param_map = gbkfit::vectors_to_map(udata->param_name, udata->param_value);


    // Provide debug information about the current set of parameters.
    std::cout << gbkfit::to_string(param_map) << std::endl;

    int free_params_counter = 0;
    for(std::size_t i = 0; i < udata->param_name.size(); ++i)
    {
        if(!udata->param_fixed[i])
        {
            udata->param_mean[i]   = paramconstr[0][npar*0+free_params_counter];
            udata->param_stddev[i] = paramconstr[0][npar*1+free_params_counter];
            udata->param_best[i]   = paramconstr[0][npar*2+free_params_counter];
            udata->param_map[i]    = paramconstr[0][npar*3+free_params_counter];
            free_params_counter++;
        }
    }
}

fitter_multinest::fitter_multinest(void)
{
}

fitter_multinest::~fitter_multinest()
{
}

const std::string& fitter_multinest::get_type_name(void) const
{
    return fitter_factory_multinest::FACTORY_TYPE_NAME;
}

void fitter_multinest::fit(model* model, const std::map<std::string,nddataset*>& datasets, parameters_fit_info& params_info) const
{
    //
    // Perform the necessary memory allocations and copy the datasets on the host.
    //

    std::vector<std::string> dataset_names;
    std::map<std::string,ndarray_host*> data_map_dat;
    std::map<std::string,ndarray_host*> data_map_msk;
    std::map<std::string,ndarray_host*> data_map_err;
    std::map<std::string,ndarray_host*> data_map_mdl;

    // Iterate over the input datasets.
    for(auto& dataset : datasets)
    {
        // Get the dataset name and store it in a vector, we will use it later for convenience.
        std::string dataset_name = std::get<0>(dataset);
        dataset_names.push_back(dataset_name);

        // Get input data pointer shortcuts for convenience.
        ndarray* input_data_dat = std::get<1>(dataset)->get_data("data");
        ndarray* input_data_msk = std::get<1>(dataset)->get_data("mask");
        ndarray* input_data_err = std::get<1>(dataset)->get_data("error");

        // Allocate host memory for the input data and the model.
        data_map_dat[dataset_name] = new ndarray_host_new(input_data_dat->get_shape());
        data_map_msk[dataset_name] = new ndarray_host_new(input_data_msk->get_shape());
        data_map_err[dataset_name] = new ndarray_host_new(input_data_err->get_shape());
        data_map_mdl[dataset_name] = new ndarray_host_new(input_data_dat->get_shape());

        // Copy data to the host.
        data_map_dat[dataset_name]->copy_data(input_data_dat);
        data_map_msk[dataset_name]->copy_data(input_data_msk);
        data_map_err[dataset_name]->copy_data(input_data_err);
    }

    //
    // Get model parameters. Always use this order for the parameters!
    //

    std::vector<std::string> param_names = model->get_parameter_names();

    //
    //  build arrays with the free parameters only
    //


    multinest_user_data udata;

    udata.param_fixed.resize(param_names.size());
    udata.param_value.resize(param_names.size());
    udata.param_min.resize(param_names.size());
    udata.param_max.resize(param_names.size());
    udata.param_mean.resize(param_names.size());
    udata.param_stddev.resize(param_names.size());
    udata.param_best.resize(param_names.size());
    udata.param_map.resize(param_names.size());

    int free_param_counter = 0;
    for(std::size_t i = 0; i < param_names.size(); ++i)
    {
        std::string param_name = param_names[i];

        udata.param_fixed[i] = params_info.get_parameter(param_name).get<bool>("fixed",0);

        if(!udata.param_fixed[i])
        {
            udata.param_min[i] = params_info.get_parameter(param_name).get<float>("min");
            udata.param_max[i] = params_info.get_parameter(param_name).get<float>("max");
            free_param_counter++;
        }
        else
        {
            udata.param_value[i] = params_info.get_parameter(param_name).get<float>("init");
            udata.param_mean[i] = udata.param_value[i];
            udata.param_stddev[i] = 0.0;
            udata.param_best[i] = udata.param_value[i];
            udata.param_map[i]= udata.param_value[i];
        }
    }

    udata.model = model;
    udata.param_name = param_names;
    udata.dataset_names = dataset_names;
    udata.data_map_dat = data_map_dat;
    udata.data_map_msk = data_map_msk;
    udata.data_map_err = data_map_err;
    udata.data_map_mdl = data_map_mdl;

    //
    // ...
    //

    int IS = 1;                         // do Nested Importance Sampling?
    int mmodal = 0;                     // do mode separation?
    int ceff = 0;                       // run in constant efficiency mode?
    int nlive = 50;                     // number of live points
    double efr = 1.0;                   // set the required efficiency
    double tol = 0.5;                   // tol, defines the stopping criteria
    int ndims = free_param_counter;     // dimensionality (no. of free parameters)
    int nPar = free_param_counter;      // total no. of parameters including free & derived parameters
    int nClsPar = free_param_counter;   // no. of parameters to do mode separation on
    int updInt = 10;                   // after how many iterations feedback is required & the output files should be updated
    double Ztol = -1E90;                // all the modes with logZ < Ztol are ignored
    int maxModes = 100;                 // expected max no. of modes (used only for memory allocation)
    int* pWrap = new int[ndims];        // which parameters to have periodic boundary conditions?
    for(int i = 0; i < ndims; i++)
        pWrap[i] = 0;
    char root[100] = "out";             // root for output files
    int seed = -1;                      // random no. generator seed, if < 0 then take the seed from system clock
    int fb = 0;                         // need feedback on standard output?
    int resume = 0;                     // resume from a previous job?
    int outfile = 0;                    // write output files?
    int initMPI = 0;                    // initialize MPI routines?, relevant only if compiling with MPI
                                        // set it to F if you want your main program to handle MPI initialization
    double logZero = -1E90;             // points with loglike < logZero will be ignored by MultiNest
    int maxiter = 2500;                 // max no. of iterations, a non-positive value means infinity. MultiNest will terminate if either it
                                        // has done max no. of iterations or convergence criterion (defined through tol) has been satisfied

    nested::run(IS,
                mmodal,
                ceff,
                nlive,
                tol,
                efr,
                ndims,
                nPar,
                nClsPar,
                maxModes,
                updInt,
                Ztol,
                root,
                seed,
                pWrap,
                fb,
                resume,
                outfile,
                initMPI,
                logZero,
                maxiter,
                multinest_callback_likelihood,
                multinest_callback_dumper,
                &udata);

    //
    // Extract fitted model parameters.
    //

    std::cout << "Fitting results:" << std::endl;
    for (std::size_t i = 0; i < param_names.size(); ++i)
    {
        std::string param_name = udata.param_name[i];
        float param_mean   = udata.param_mean[i];
        float param_stddev = udata.param_stddev[i];
        float param_best   = udata.param_best[i];
        float param_map    = udata.param_map[i];

        std::cout << std::fixed
                  << std::setprecision(2)
                  << "Fitted param:"
                  << " name="   << std::setw(4) << param_name   << ","
                  << " mean="   << std::setw(8) << param_mean   << ","
                  << " stddev=" << std::setw(8) << param_stddev << ","
                  << " best="   << std::setw(8) << param_best   << ","
                  << " map="    << std::setw(8) << param_map    << std::endl;
    }

    //
    // Perform clean up
    //

    delete [] pWrap;
    for(auto& data : data_map_mdl) delete std::get<1>(data);
    for(auto& data : data_map_dat) delete std::get<1>(data);
    for(auto& data : data_map_msk) delete std::get<1>(data);
    for(auto& data : data_map_err) delete std::get<1>(data);
}

const std::string fitter_factory_multinest::FACTORY_TYPE_NAME = "gbkfit.fitters.multinest";

fitter_factory_multinest::fitter_factory_multinest(void)
{
}

fitter_factory_multinest::~fitter_factory_multinest()
{
}

const std::string& fitter_factory_multinest::get_type_name(void) const
{
    return FACTORY_TYPE_NAME;
}

fitter* fitter_factory_multinest::create_fitter(const std::string& info) const
{
    // Parse input string as xml.
    std::stringstream info_stream(info);
    boost::property_tree::ptree info_ptree;
    boost::property_tree::read_xml(info_stream,info_ptree);

    return new fitter_multinest();
}


} // namespace multinest
} // namespace fitters
} // namespace gbkfit
