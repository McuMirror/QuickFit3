#ifndef QFFitFunctionsSPIMFCCSDiff_H
#define QFFitFunctionsSPIMFCCSDiff_H
#include "qfpluginfitfunction.h"




/*! \brief QFFitFunction class for a SPIM-FCCS fit model with pure diffusion and flow and 1/sqrt(e) lateral width
    \ingroup qf3fitfunp_fitfunctions_spimfcs
*/
class QFFitFunctionsSPIMFCCSDiff: public QFFitFunction {
    public:
        QFFitFunctionsSPIMFCCSDiff();
        virtual ~QFFitFunctionsSPIMFCCSDiff() {}
        /*! \copydoc QFFitFunction::name()   */
        virtual QString name() const { return QString("SPIM-FCCS: Diffusion, 2-pixel crosscorrelation (1/sqrt(e) radii)"); };
        /** \copydoc QFFitFunction::shortName() */
        virtual QString shortName() const { return QObject::tr("SPIM-FCCS: Diffusion (1/sqrt(e) radii)"); };
        /*! \copydoc QFFitFunction::id()   */
        virtual QString id() const { return QString("fccs_spim_diff"); };

        /*! \copydoc QFFitFunction::evaluate()   */
        virtual double evaluate(double t, const double* parameters) const;

        /*! \copydoc QFFitFunction::calcParameter()   */
        virtual void calcParameter(double* parameterValues, double* error=NULL) const;

        /*! \copydoc QFFitFunction::isParameterVisible()   */
        virtual bool isParameterVisible(int parameter, const double* parameterValues) const;
        /*! \copydoc QFFitFunction::getAdditionalPlotCount()   */
        virtual unsigned int getAdditionalPlotCount(const double* params);

        /*! \copydoc QFFitFunction::transformParametersForAdditionalPlot()   */
        virtual QString transformParametersForAdditionalPlot(int plot, double* params);

        /*! \copydoc QFFitFunction::get_implementsDerivatives()   */
        virtual bool get_implementsDerivatives() { return false; };
};

#endif // QFFitFunctionsSPIMFCCSDiff_H