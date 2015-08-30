
#include "application.hpp"

#include "gbkfit/core.hpp"
#include "gbkfit/fits.hpp"
#include "gbkfit/ndarray_host.hpp"
#include "gbkfit/nddataset.hpp"

#include "gbkfit/fitters/mpfit/fitter_mpfit.hpp"
#include "gbkfit/fitters/multinest/fitter_multinest.hpp"
#include "gbkfit/models/galaxy_2d/model_galaxy_2d.hpp"
#include "gbkfit/models/galaxy_2d_omp/model_galaxy_2d_omp.hpp"
#include "gbkfit/models/galaxy_2d_cuda/model_galaxy_2d_cuda.hpp"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "gbkfit/spread_function.hpp"
#include "gbkfit/utility.hpp"
#include "gbkfit/parameters_fit_info.hpp"

namespace gbkfit_app_cli {


application::application(void)
    : m_core(nullptr)
    , m_model_factory_galaxy_2d_cuda(nullptr)
    , m_model_factory_galaxy_2d_omp(nullptr)
    , m_fitter_factory_mpfit(nullptr)
    , m_fitter_factory_multinest(nullptr)
    , m_model(nullptr)
    , m_fitter(nullptr)
    , m_fit_info(nullptr)
{
}

application::~application()
{
}

int* gdata = nullptr;
char* gdata2 = nullptr;

template<typename T>
void as(T* data)
{
    std::copy(gdata,gdata+1,data);
}

bool application::initialize(void)
{
    std::cout << "Initialization started." << std::endl;

    // create gbkfit core
    m_core = new gbkfit::core();

    // create model factories
    m_model_factory_galaxy_2d_cuda = new gbkfit::models::galaxy_2d::model_factory_galaxy_2d_cuda();
    m_model_factory_galaxy_2d_omp = new gbkfit::models::galaxy_2d::model_factory_galaxy_2d_omp();

    // create fitter factories
    m_fitter_factory_mpfit = new gbkfit::fitters::mpfit::fitter_factory_mpfit();
    m_fitter_factory_multinest = new gbkfit::fitters::multinest::fitter_factory_multinest();

    // add model factories to the gbkfit core
    m_core->add_model_factory(m_model_factory_galaxy_2d_cuda);
    m_core->add_model_factory(m_model_factory_galaxy_2d_omp);

    // add fitter factories to the gbkfit core
    m_core->add_fitter_factory(m_fitter_factory_mpfit);
    m_core->add_fitter_factory(m_fitter_factory_multinest);

    // read configuration
    boost::property_tree::ptree ptree_config;
    boost::property_tree::read_xml("../../tools/gbkfit_config.xml",ptree_config);

    // get model info
    std::stringstream model_info;
    boost::property_tree::write_xml(model_info,ptree_config.get_child("gbkfit.model"));

    // get fitter info
    std::stringstream fitter_info;
    boost::property_tree::write_xml(fitter_info,ptree_config.get_child("gbkfit.fitter"));

    // ...
    std::stringstream parameters_info;
    boost::property_tree::write_xml(parameters_info,ptree_config.get_child("gbkfit.parameters"));

    // get detasets info
    std::stringstream datasets_info;
    boost::property_tree::write_xml(datasets_info,ptree_config.get_child("gbkfit.datasets"));

    // create datasets
    m_datasets = m_core->create_datasets(datasets_info.str());

    // create model
    m_model = m_core->create_model(model_info.str());

    //m_model = m_core->create_model(model_info.str());

    // create fitter
//  m_fitter = m_core->create_fitter(fitter_info.str());

    m_fit_info = m_core->create_parameters_fit_info(parameters_info.str());

    std::cout << "Initialization completed." << std::endl;

    return true;
}

void application::shutdown(void)
{
    std::cout << "Shutdown started." << std::endl;

    delete m_core;
    delete m_model_factory_galaxy_2d_cuda;
    delete m_model_factory_galaxy_2d_omp;
    delete m_fitter_factory_mpfit;
    delete m_fitter_factory_multinest;
    delete m_model;
    delete m_fitter;
    delete m_fit_info;

    for(auto& dataset : m_datasets)
    {
        dataset.second->__destroy();
        delete dataset.second;
    }

    std::cout << "Shutdown completed." << std::endl;
}

void application::run(void)
{
    std::cout << "Main loop started." << std::endl;

    gbkfit::parameters_fit_info foo;


    foo.add_parameter("xo").add<float>("option1",4.0);

    auto foo1 = foo.get_parameter("xo").get<int>("option1");

    std::cout << foo1 << std::endl;




    //gbkfit::model_parameters_fit_info::model_parameter_fit_info foo1;
    //foo.add_parameter("xo").add("init",10).add("min",10);




    if(m_model)
    {
        std::map<std::string,float> params = {
            {"vsys",0},
            {"xo",8},
            {"yo",8},
            {"pa",90},
            {"incl",45},
            {"i0",1.0},
            {"r0",2.0},
            {"rt",5.0},
            {"vt",200},
            {"vsig",40}
        };

        m_model->get_parameter_names();

    //  std::vector<gbkfit::ndarray*> data = m_model->evaluate(params);
        std::map<std::string,gbkfit::ndarray*> data = m_model->get_data();
        gbkfit::fits::write_to("!foo_flxmap.fits",*data["flxmap"]);
        gbkfit::fits::write_to("!foo_velmap.fits",*data["velmap"]);
        gbkfit::fits::write_to("!foo_sigmap.fits",*data["sigmap"]);
    }

    std::cout << "Main loop finished." << std::endl;
}


} // namespace gbkfit_app_cli
