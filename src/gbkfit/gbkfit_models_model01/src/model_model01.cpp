
#include "gbkfit/models/model01/model_model01.hpp"

namespace gbkfit {
namespace models {
namespace model01 {

model_model01::model_model01(profile_flx_type profile_flx, profile_vel_type profile_vel)
    : m_profile_flx(profile_flx)
    , m_profile_vel(profile_vel)
{
    // Projection parameters.
    m_parameter_names.push_back("xo");
    m_parameter_names.push_back("yo");
    m_parameter_names.push_back("pa");
    m_parameter_names.push_back("incl");

    // Flux model parameters.
    if (m_profile_flx == exponential) {
        m_parameter_names.push_back("i0");
        m_parameter_names.push_back("r0");
    }

    // Rotation curve model parameters.
    if (m_profile_vel == lramp) {
        m_parameter_names.push_back("rt");
        m_parameter_names.push_back("vt");
    }
    else if (m_profile_vel == arctan) {
        m_parameter_names.push_back("rt");
        m_parameter_names.push_back("vt");
    }
    else if (m_profile_vel == epinat) {
        m_parameter_names.push_back("rt");
        m_parameter_names.push_back("vt");
        m_parameter_names.push_back("a");
        m_parameter_names.push_back("b");
    }

    // Systemic velocity parameter.
    m_parameter_names.push_back("vsys");

    // Velocity dispersion model parameters.
    m_parameter_names.push_back("vsig");
}

model_model01::~model_model01()
{
}

const std::vector<std::string>& model_model01::get_parameter_names(void) const
{
    return m_parameter_names;
}

const std::vector<float>& model_model01::get_parameter_values(void) const
{
    return m_parameter_values;
}

const std::map<std::string,ndarray*>& model_model01::evaluate(const std::map<std::string,float>& parameters)
{
    // Projection parameters.
    std::vector<float> params_prj;
    params_prj.push_back(parameters.at("xo"));
    params_prj.push_back(parameters.at("yo"));
    params_prj.push_back(parameters.at("pa"));
    params_prj.push_back(parameters.at("incl"));

    // Flux model parameters.
    std::vector<float> params_flx;
    if (m_profile_flx == exponential) {
        params_flx.push_back(parameters.at("i0"));
        params_flx.push_back(parameters.at("r0"));
    }

    // Rotation curve model parameters.
    std::vector<float> params_vel;
    if (m_profile_vel == lramp) {
        params_vel.push_back(parameters.at("rt"));
        params_vel.push_back(parameters.at("vt"));
    }
    else if (m_profile_vel == arctan) {
        params_vel.push_back(parameters.at("rt"));
        params_vel.push_back(parameters.at("vt"));
    }
    else if (m_profile_vel == epinat) {
        params_vel.push_back(parameters.at("rt"));
        params_vel.push_back(parameters.at("vt"));
        params_vel.push_back(parameters.at("a"));
        params_vel.push_back(parameters.at("b"));
    }

    // Systemic velocity parameter.
    float param_vsys = parameters.at("vsys");

    // Velocity dispersion model parameters.
    float param_vsig = parameters.at("vsig");

    // ...
    return evaluate(m_profile_flx, m_profile_vel, params_prj, params_flx, params_vel, param_vsys, param_vsig);
}

model_factory_model01::model_factory_model01(void)
{
}

model_factory_model01::~model_factory_model01()
{
}

} // namespace model01
} // namespace models
} // namespace gbkfit
