#include "qffcsfitevaluationeditor.h"
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include "qfrawdatarecord.h"
#include "qfevaluationitem.h"
#include "../interfaces/qfrdrfcsdatainterface.h"
#include <iostream>
#include <cfloat>
#include "qffcsfitevaluation.h"
#include "tools.h"
#include "statistics_tools.h"
#include <QThread>
#include "dlgestimatefocalvolume.h"
#include "qmoretextobject.h"
#include "qmodernprogresswidget.h"
#include "qffitfunctionmanager.h"
#include "qffitalgorithmmanager.h"






QFFCSFitEvaluationEditor::QFFCSFitEvaluationEditor(QFPluginServices* services, QFEvaluationPropertyEditor* propEditor, QWidget* parent):
    QFFitResultsByIndexEvaluationEditorWithWidgets("fcsfitevaleditor/", propEditor, services, parent, true, true)
{
    createWidgets();
}

QFFCSFitEvaluationEditor::~QFFCSFitEvaluationEditor()
{
    //dtor
}


void QFFCSFitEvaluationEditor::createWidgets() {
    cmbWeights=new QComboBox(this);
    cmbWeights->setEditable(false);
    cmbWeights->addItem(tr("equal weights"));
    cmbWeights->addItem(tr("standard deviation"));
    cmbWeights->addItem(tr("per run errors"));
    cmbWeights->setMaximumWidth(150);
    cmbWeights->setMinimumWidth(150);
    QLabel* l=new QLabel(tr("&Weight Model: "), this);
    l->setBuddy(cmbWeights);
    layAlgorithm->addSpacing(32);
    layAlgorithm->addWidget(l);
    layAlgorithm->addWidget(cmbWeights);
    layAlgorithm->addStretch();

    btnCalibrateFocalVolume=createButtonAndActionShowText(actCalibrateFocalVolume, QIcon(":/fcsfit/focalvolume.png"), tr("Focal &Volume"), this);
    actCalibrateFocalVolume->setToolTip(tr("estimate the focal volume from a given concentration or diffusion coefficient"));
    layButtons->addWidget(btnCalibrateFocalVolume, 8, 0);
    connect(actCalibrateFocalVolume, SIGNAL(triggered()), this, SLOT(calibrateFocalVolume()));

    menuFit->addSeparator();
    menuFit->addAction(actCalibrateFocalVolume);
}


void QFFCSFitEvaluationEditor::connectWidgets(QFEvaluationItem* current, QFEvaluationItem* old) {
    QFFitResultsByIndexEvaluationEditorWithWidgets::connectDefaultWidgets(current, old, false);

    QFFCSFitEvaluation* fcs=qobject_cast<QFFCSFitEvaluation*>(current);

    if (old!=NULL) {
        disconnect(cmbWeights, SIGNAL(currentIndexChanged(int)), this, SLOT(weightsChanged(int)));
    }



    if (fcs) {

        dataEventsEnabled=false;
        cmbWeights->setCurrentIndex(current->getProperty("weights", 0).toInt());
        dataEventsEnabled=true;
    }

    connect(cmbWeights, SIGNAL(currentIndexChanged(int)), this, SLOT(weightsChanged(int)));

    displayModel(true);
    replotData();
}


void QFFCSFitEvaluationEditor::readSettings() {
    QFFitResultsByIndexEvaluationEditorWithWidgets::readSettings();
}

void QFFCSFitEvaluationEditor::writeSettings() {
    QFFitResultsByIndexEvaluationEditorWithWidgets::writeSettings();
    settings->getQSettings()->setValue(QString("fitevaleditor_%1%2/weights").arg(current->getType()).arg(current->getID()), cmbWeights->currentIndex());

}

void QFFCSFitEvaluationEditor::highlightingChanged(QFRawDataRecord* formerRecord, QFRawDataRecord* currentRecord) {
    QFFitResultsByIndexEvaluationEditorWithWidgets::highlightingChanged(formerRecord, currentRecord);
    QFFCSFitEvaluation* eval=qobject_cast<QFFCSFitEvaluation*>(currentRecord);
    if (eval) {
        dataEventsEnabled=false;
        cmbWeights->setCurrentIndex(eval->getFitDataWeighting());
        dataEventsEnabled=true;
    }
}



void QFFCSFitEvaluationEditor::displayModel(bool newWidget) {
    QFFitResultsByIndexEvaluationEditorWithWidgets::displayModel(newWidget);
    QFFCSFitEvaluation* eval=qobject_cast<QFFCSFitEvaluation*>(current);
    if (newWidget && eval) {
        QFFitFunction* ffunc=eval->getFitFunction();
        if (ffunc) {

            /////////////////////////////////////////////////////////////////////////////////////////////
            // create new parameter widgets
            /////////////////////////////////////////////////////////////////////////////////////////////
            actCalibrateFocalVolume->setEnabled(false);
            bool has_particles=false;
            bool has_wxy=false;
            bool has_tauD=false;
            bool has_gamma=false;
            for (int i=0; i<ffunc->paramCount(); i++) {
                QString id=ffunc->getParameterID(i);
                if ((id.toLower()=="n_particle") || (id.toLower()=="1n_particle")) has_particles=true;
                if ((id.toLower()=="diff_tau1") || (id.toLower()=="diff_tau")) has_tauD=true;
                if ((id.toLower()=="focus_width")) has_wxy=true;
                if ((id.toLower()=="focus_struct_fac")) has_gamma=true;

            }
            actCalibrateFocalVolume->setEnabled((has_tauD||(has_particles&&has_gamma))&&has_wxy);
        }
    }
}



void QFFCSFitEvaluationEditor::updateFitFunctions() {
    if (!current) return;
    if (!cmbModel) return;
    QFRawDataRecord* record=current->getHighlightedRecord();
    QFRDRFCSDataInterface* data=qobject_cast<QFRDRFCSDataInterface*>(record);
    QFFCSFitEvaluation* eval=qobject_cast<QFFCSFitEvaluation*>(current);
    int run=eval->getCurrentIndex();
    JKQTPdatastore* ds=pltData->getDatastore();
    JKQTPdatastore* dsres=pltResiduals->getDatastore();
    JKQTPdatastore* dsresh=pltResidualHistogram->getDatastore();
    JKQTPdatastore* dsresc=pltResidualCorrelation->getDatastore();
    QFFitFunction* ffunc=eval->getFitFunction();
    //QFFCSFitEvaluation::DataWeight weighting=eval->getFitDataWeighting();

    if (!ffunc) return;

    if ((!eval)||(!data)) {
        return;
    }


    //qDebug()<<" **** updateFitFunctions";
    QTime t, t1;
    t.start();
    t1.start();

    int residualStyle=cmbResidualStyle->currentIndex();
    int residualHistogramBins=spinResidualHistogramBins->value();
    int datacut_min=datacut->get_userMin();
    int datacut_max=datacut->get_userMax();


    //bool updEn=updatesEnabled();
    //setUpdatesEnabled(false);
    try {

        if (data->getCorrelationN()>0) {
            int c_tau=ds->getColumnNum("tau");
            if (c_tau>=0) { // we only add a graph, if we have a column with tau values
                eval->set_doEmitPropertiesChanged(false);
                eval->set_doEmitResultsChanged(false);
                record->disableEmitResultsChanged();
                //qDebug()<<"    a "<<t.elapsed()<<" ms";
                t.start();


                /////////////////////////////////////////////////////////////////////////////////
                // retrieve data and tau-values from rawdata record
                /////////////////////////////////////////////////////////////////////////////////
                long N=data->getCorrelationN();
                int runAvgWidth=11;
                double* tauvals=data->getCorrelationT();
                double* corrdata=NULL;

                if (eval->getCurrentIndex()<0) {
                    corrdata=data->getCorrelationMean();
                } else {
                    if (eval->getCurrentIndex()<(int)data->getCorrelationRuns()) {
                        corrdata=data->getCorrelationRun(eval->getCurrentIndex());
                    } else {
                        corrdata=data->getCorrelationMean();
                    }
                }

                double* weights=eval->allocWeights(NULL, record, eval->getCurrentIndex(), datacut_min, datacut_max);


                /////////////////////////////////////////////////////////////////////////////////
                // retrieve fit parameters and errors. run calcParameters to fill in calculated parameters and make sure
                // we are working with a complete set of parameters
                /////////////////////////////////////////////////////////////////////////////////
                double* fullParams=eval->allocFillParameters();
                double* errors=eval->allocFillParameterErrors();
                bool* paramsFix=eval->allocFillFix(record, eval->getCurrentIndex());
                ffunc->calcParameter(fullParams, errors);

                /////////////////////////////////////////////////////////////////////////////////
                // calculate fit statistics
                /////////////////////////////////////////////////////////////////////////////////
                record->disableEmitResultsChanged();
                QFFitStatistics fitResults=eval->calcFitStatistics(eval->hasFit(record, run), ffunc, N, tauvals, corrdata, weights, datacut_min, datacut_max, fullParams, errors, paramsFix, runAvgWidth, residualHistogramBins, record, run);
                record->enableEmitResultsChanged();


                size_t c_fit = ds->addCopiedColumn(fitResults.fitfunc, N, "fit_model");
                //qDebug()<<"    f "<<t.elapsed()<<" ms";
                t.start();

                /////////////////////////////////////////////////////////////////////////////////
                // plot fit model and additional function graphs
                /////////////////////////////////////////////////////////////////////////////////
                JKQTPxyLineGraph* g_fit=new JKQTPxyLineGraph(pltData->get_plotter());
                g_fit->set_drawLine(true);
                g_fit->set_title("fit function");
                g_fit->set_xColumn(c_tau);
                g_fit->set_yColumn(c_fit);
                g_fit->set_datarange_start(datacut->get_userMin());
                g_fit->set_datarange_end(datacut->get_userMax());
                for (int i=0; i<(int)ffunc->getAdditionalPlotCount(fullParams); i++) {
                    double* params=eval->allocFillParameters();
                    QString name=ffunc->transformParametersForAdditionalPlot(i, params);
                    double* afitfunc=(double*)malloc(N*sizeof(double));
                    for (int j=0; j<N; j++) {
                        afitfunc[j]=ffunc->evaluate(tauvals[j], params);
                    }
                    size_t c_afit=ds->addCopiedColumn(afitfunc, N, QString("add_fit_model_%1").arg(i));
                    JKQTPxyLineGraph* g_afit=new JKQTPxyLineGraph(pltData->get_plotter());
                    g_afit->set_drawLine(true);
                    g_afit->set_title(name);
                    g_afit->set_xColumn(c_tau);
                    g_afit->set_yColumn(c_afit);
                    g_afit->set_datarange_start(datacut->get_userMin());
                    g_afit->set_datarange_end(datacut->get_userMax());
                    pltData->addGraph(g_afit);
                    free(params);
                    free(afitfunc);
                }
                pltData->addGraph(g_fit);
                //qDebug()<<"    g "<<t.elapsed()<<" ms";
                t.start();


                /////////////////////////////////////////////////////////////////////////////////
                // plot residuals
                /////////////////////////////////////////////////////////////////////////////////
                size_t c_taures=c_tau;//dsres->addCopiedColumn(data->getCorrelationT(), N, "tau");
                size_t c_residuals=0;
                JKQTPxyLineGraph* g_residuals=new JKQTPxyLineGraph(pltResiduals->get_plotter());
                if (chkWeightedResiduals->isChecked()) {
                    c_residuals=dsres->addCopiedColumn(fitResults.residuals_weighted, N, "residuals_weighted");
                    g_residuals->set_title("weighted residuals");
                } else {
                    c_residuals=dsres->addCopiedColumn(fitResults.residuals, N, "residuals");
                    g_residuals->set_title("residuals");
                }
                g_residuals->set_xColumn(c_taures);
                g_residuals->set_yColumn(c_residuals);
                g_residuals->set_symbolSize(8);
                g_residuals->set_symbolWidth(1);
                g_residuals->set_datarange_start(datacut->get_userMin());
                g_residuals->set_datarange_end(datacut->get_userMax());
                g_residuals->set_drawLine(true);
                if (residualStyle==0) { // draw points
                    g_residuals->set_drawLine(false);
                    g_residuals->set_symbol(JKQTPcross);
                } else if (residualStyle==2) {
                    g_residuals->set_symbol(JKQTPcross);
                }
                pltResiduals->addGraph(g_residuals);
                //qDebug()<<"    h "<<t.elapsed()<<" ms";
                t.start();


                /////////////////////////////////////////////////////////////////////////////////
                // plot residuals running average
                /////////////////////////////////////////////////////////////////////////////////
                size_t c_tauresra=dsres->addCopiedColumn(fitResults.tau_runavg, fitResults.runAvgN, "tau_resid_runavg");
                size_t c_residualsra=0;
                JKQTPxyLineGraph* g_residualsra=new JKQTPxyLineGraph(pltResiduals->get_plotter());


                if (chkWeightedResiduals->isChecked()) {
                    c_residualsra=dsres->addCopiedColumn(fitResults.residuals_runavg_weighted, fitResults.runAvgN, "residuals_runavg_weighted");
                    g_residualsra->set_title("weighted residuals, movAvg");
                } else {
                    c_residualsra=dsres->addCopiedColumn(fitResults.residuals_runavg, fitResults.runAvgN, "residuals_runavg");
                    g_residualsra->set_title("residuals, movAvg");
                }
                g_residualsra->set_xColumn(c_tauresra);
                g_residualsra->set_yColumn(c_residualsra);
                g_residualsra->set_symbol(JKQTPnoSymbol);
                g_residualsra->set_symbolSize(6);
                g_residualsra->set_symbolWidth(1);
                g_residualsra->set_color(g_residuals->get_color().darker());
                //g_residuals->set_datarange_start(datacut->get_userMin());
                //g_residuals->set_datarange_end(datacut->get_userMax());
                g_residualsra->set_drawLine(true);
                /*if (residualStyle==0) { // draw points
                    // always draw as lines
                    g_residualsra->set_symbol(JKQTPplus);
                } else if (residualStyle==2) {
                    g_residualsra->set_symbol(JKQTPplus);
                }*/
                pltResiduals->addGraph(g_residualsra);
                //qDebug()<<"    i "<<t.elapsed()<<" ms";
                t.start();

                /////////////////////////////////////////////////////////////////////////////////
                // plot residuals histogram
                /////////////////////////////////////////////////////////////////////////////////
                size_t c_residualHistogramX=0;
                size_t c_residualHistogramY=0;
                if (chkWeightedResiduals->isChecked()) {
                    c_residualHistogramX=dsresh->addLinearColumn(residualHistogramBins, fitResults.rminw+fitResults.residHistWBinWidth/2.0, fitResults.rmaxw-fitResults.residHistWBinWidth/2.0, "residualhist_weighted_x");
                    c_residualHistogramY=dsresh->addCopiedColumn(fitResults.resWHistogram, residualHistogramBins, "residualhist_weighted_y");
                } else {
                    c_residualHistogramX=dsresh->addLinearColumn(residualHistogramBins, fitResults.rmin+fitResults.residHistBinWidth/2.0, fitResults.rmax-fitResults.residHistBinWidth/2.0, "residualhist_x");
                    c_residualHistogramY=dsresh->addCopiedColumn(fitResults.resHistogram, residualHistogramBins, "residualhist_y");
                }
                JKQTPbarHorizontalGraph* g_residualsHistogram=new JKQTPbarHorizontalGraph(pltResidualHistogram->get_plotter());
                g_residualsHistogram->set_xColumn(c_residualHistogramX);
                g_residualsHistogram->set_yColumn(c_residualHistogramY);
                g_residualsHistogram->set_shift(0);
                g_residualsHistogram->set_width(1.0);
                pltResidualHistogram->addGraph(g_residualsHistogram);
                //qDebug()<<"    j "<<t.elapsed()<<" ms";
                t.start();


                /////////////////////////////////////////////////////////////////////////////////
                // plot residuals correlations
                /////////////////////////////////////////////////////////////////////////////////
                size_t c_residualCorrelationX=dsresc->addLinearColumn(fitResults.resN-1, 1, fitResults.resN-1, "residualcorr_x");
                size_t c_residualCorrelationY=0;
                if (chkWeightedResiduals->isChecked()) {
                    c_residualCorrelationY=dsresc->addCopiedColumn(&(fitResults.resWCorrelation[1]), fitResults.resN-1, "residualcorr_weighted_y");
                } else {
                    c_residualCorrelationY=dsresh->addCopiedColumn(&(fitResults.resCorrelation[1]), fitResults.resN-1, "residualcorr_y");
                }
                JKQTPxyLineGraph* g_residualsCorrelation=new JKQTPxyLineGraph(pltResidualCorrelation->get_plotter());
                g_residualsCorrelation->set_xColumn(c_residualCorrelationX);
                g_residualsCorrelation->set_yColumn(c_residualCorrelationY);
                pltResidualCorrelation->addGraph(g_residualsCorrelation);
                //qDebug()<<"    k "<<t.elapsed()<<" ms";
                t.start();

                /////////////////////////////////////////////////////////////////////////////////
                // update display of fit results
                /////////////////////////////////////////////////////////////////////////////////
                QString txtFit="<font face=\"Arial\">";
                QString fitResult=record->resultsGetAsString(eval->getEvaluationResultID(record), "fitalg_messageHTML");

                if (!fitResult.isEmpty()) {
                    txtFit+=txtFit+tr("<div style=\"border-style:solid\"><b>Fit Result Message:</b><center>%1</center></div><br>").arg(fitResult);
                } else {
                    txtFit+=txtFit+tr("<div style=\"border-style:solid\"><b>Fit Result Message:</b><center>not fit yet</center></div><br>");
                }
                txtFit+=QString("<b>%1</b><cebter>").arg(tr("Fit Statistics:"));
                txtFit+=QString("<table border=\"0\" width=\"95%\">");
                //txtFit+=QString("<tr><td align=\"right\"></td><td align=\"left\"></td><td align=\"right\"></td><td align=\"left\"></td></tr>");
                txtFit+=QString("<tr>"
                                "<td align=\"right\" valign=\"bottom\"><font size=\"+2\">&chi;<sup>2</sup></font> =</td><td align=\"left\" valign=\"bottom\">%1</td>"
                                "<td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td>"
                                "<td align=\"right\" valign=\"bottom\"><font size=\"+2\">&chi;<sup>2</sup></font> (weighted) =</td><td align=\"left\" valign=\"bottom\">%2</td>"
                                "</tr>").arg(fitResults.residSqrSum).arg(fitResults.residWeightSqrSum);
                txtFit+=QString("<tr>"
                                "<td align=\"right\" valign=\"bottom\">&lang;E&rang;=</td><td align=\"left\" valign=\"bottom\">%1</td>"
                                "<td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td>"
                                "<td align=\"right\" valign=\"bottom\"> &lang;E&rang; (weighted) =</td><td align=\"left\" valign=\"bottom\">%2</td>"
                                "</tr>").arg(fitResults.residAverage).arg(fitResults.residWeightAverage);
                txtFit+=QString("<tr>"
                                "<td align=\"right\" valign=\"bottom\">&radic;&lang;E<sup><font size=\"+1\">2</font></sup>&rang;=</td><td align=\"left\" valign=\"bottom\">%1</td>"
                                "<td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td>"
                                "<td align=\"right\" valign=\"bottom\"> &radic;&lang;E<sup><font size=\"+1\">2</font></sup>&rang; (weighted) =</td><td align=\"left\" valign=\"bottom\">%2</td>"
                                "</tr>").arg(fitResults.residStdDev).arg(fitResults.residWeightStdDev);
                txtFit+=QString("<tr>"
                                "<td align=\"right\" valign=\"bottom\">NP =</td><td align=\"left\" valign=\"bottom\">%1</td>"
                                "<td></td>"
                                "<td align=\"right\" valign=\"bottom\">NR =</td><td align=\"left\" valign=\"bottom\">%2</td>"
                                "</tr>").arg(fitResults.fitparamN).arg(fitResults.dataSize);
                txtFit+=QString("<tr>"
                                "<td align=\"right\" valign=\"bottom\">DF =</td><td align=\"left\" valign=\"bottom\">%1</td>"
                                "<td></td>"
                                "<td align=\"right\" valign=\"bottom\"></td><td align=\"left\" valign=\"bottom\"></td>"
                                "</tr>").arg(fitResults.degFreedom);
                txtFit+=QString("<tr>"
                                "<td align=\"right\" valign=\"bottom\">TSS  =</td><td align=\"left\" valign=\"bottom\">%1</td>"
                                "<td></td>"
                                "<td align=\"right\" valign=\"bottom\">R<sup>2</sup> =</td><td align=\"left\" valign=\"bottom\">%2</td>"
                                "</tr>").arg(fitResults.TSS).arg(fitResults.Rsquared);
                //qDebug()<<"    l "<<t.elapsed()<<" ms";
                t.start();

                txtFit+=QString("</table><br><font size=\"-1\"><i>Legend:</i>: &chi;<sup>2</sup>: sum error square, &lang;E&rang;: residual average, &radic;&lang;E2&rang;: residual stddev., <br>NP: number of fit parameters, NR: number of residuals, <br>DF: degrees of freedom, R<sup>2</sup>: coefficient of determination, <br>TSS: total sum of squares</font>");
                txtFit+=QString("</center></font>");
                fitStatisticsReport=txtFit;
                txtFitStatistics->setHtml(txtFit);

                //qDebug()<<"    m_presignals "<<t.elapsed()<<" ms";
                eval->set_doEmitPropertiesChanged(true);
                eval->set_doEmitResultsChanged(true);
                //record->enableEmitResultsChanged();

                //qDebug()<<"    m "<<t.elapsed()<<" ms";
                t.start();


                /////////////////////////////////////////////////////////////////////////////////
                // clean memory
                /////////////////////////////////////////////////////////////////////////////////
                free(fullParams);
                free(errors);
                free(weights);
                free(paramsFix);
                fitResults.free();

                //qDebug()<<"    n "<<t.elapsed()<<" ms";
                t.start();
            }
        }
    } catch(std::exception& E) {
        services->log_error(tr("error during plotting, error message: %1\n").arg(E.what()));
    }
    //setUpdatesEnabled(updEn);
    //qDebug()<<"    updateFitFunctions end   runtime = "<<t1.elapsed()<<"ms";

}



























void QFFCSFitEvaluationEditor::weightsChanged(int model) {
    if (!dataEventsEnabled) return;
    if (!current) return;
    if (!current->getHighlightedRecord()) return;
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    current->setQFProperty("weights", cmbWeights->currentIndex(), false, false);
    QFFCSFitEvaluation* data=qobject_cast<QFFCSFitEvaluation*>(current);
    if (data) {
        if (cmbWeights->currentIndex()==0) data->setFitDataWeighting(QFFCSFitEvaluation::EqualWeighting);
        else if (cmbWeights->currentIndex()==1) data->setFitDataWeighting(QFFCSFitEvaluation::StdDevWeighting);
        else if (cmbWeights->currentIndex()==2) data->setFitDataWeighting(QFFCSFitEvaluation::RunErrorWeighting);
        else data->setFitDataWeighting(QFFCSFitEvaluation::EqualWeighting);
    }
    displayModel(true);
    replotData();
    QApplication::restoreOverrideCursor();
}



























int QFFCSFitEvaluationEditor::getUserMin(QFRawDataRecord* rec, int index) {
    return getUserMin(rec, index, datacut->get_userMin());
}

int QFFCSFitEvaluationEditor::getUserMax(QFRawDataRecord* rec, int index) {
    return getUserMax(rec, index, datacut->get_userMax());
}

int QFFCSFitEvaluationEditor::getUserMin() {
    return getUserMin(datacut->get_userMin());
}

int QFFCSFitEvaluationEditor::getUserMax() {
    return getUserMax(datacut->get_userMax());
}

int QFFCSFitEvaluationEditor::getUserRangeMax(QFRawDataRecord *rec, int index) {
    QFRDRFCSDataInterface* data=qobject_cast<QFRDRFCSDataInterface*>(rec);
    if (data) {
        return data->getCorrelationN()-1;
    }
    return 0;
}

int QFFCSFitEvaluationEditor::getUserRangeMin(QFRawDataRecord *rec, int index) {
    return 0;
}


void QFFCSFitEvaluationEditor::replotData() {


    if (!current) return;
    if (!cmbModel) return;
    QFRawDataRecord* record=current->getHighlightedRecord();
    QFRDRFCSDataInterface* data=qobject_cast<QFRDRFCSDataInterface*>(record);
    QFFCSFitEvaluation* eval=qobject_cast<QFFCSFitEvaluation*>(current);
    JKQTPdatastore* ds=pltData->getDatastore();
    JKQTPdatastore* dsres=pltResiduals->getDatastore();
    JKQTPdatastore* dsresh=pltResidualHistogram->getDatastore();
    JKQTPdatastore* dsresc=pltResidualCorrelation->getDatastore();

    if ((!eval)||(!data)) {
        pltData->clearGraphs();
        ds->clear();
        return;
    }

    //qDebug()<<" **** replotData()";
    QTime t, t1;
    t.start();
    t1.start();

    //bool updEn=updatesEnabled();
    //setUpdatesEnabled(false);

    pltResiduals->set_doDrawing(false);
    pltResiduals->set_emitSignals(false);
    pltResiduals->clearGraphs();
    pltData->set_doDrawing(false);
    pltData->set_emitSignals(false);
    pltData->clearGraphs();
    pltResidualHistogram->set_doDrawing(false);
    pltResidualHistogram->set_emitSignals(false);
    pltResidualHistogram->clearGraphs();
    pltResidualCorrelation->set_doDrawing(false);
    pltResidualCorrelation->set_emitSignals(false);
    pltResidualCorrelation->clearGraphs();
    dsres->clear();
    ds->clear();
    dsresh->clear();
    dsresc->clear();

    //qDebug()<<"   a "<<t.elapsed()<<" ms";
    t.start();

    pltResiduals->getXAxis()->set_logAxis(chkXLogScale->isChecked());
    pltData->getXAxis()->set_logAxis(chkXLogScale->isChecked());
    if (chkXLogScale->isChecked()) {
        pltData->getXAxis()->set_minorTicks(9);
        pltResiduals->getXAxis()->set_minorTicks(9);
    } else {
        pltData->getXAxis()->set_minorTicks(1);
        pltResiduals->getXAxis()->set_minorTicks(1);
    }
    pltResiduals->getXAxis()->set_drawGrid(chkGrid->isChecked());
    pltResiduals->getYAxis()->set_drawGrid(chkGrid->isChecked());
    pltData->getXAxis()->set_drawGrid(chkGrid->isChecked());
    pltData->getYAxis()->set_drawGrid(chkGrid->isChecked());
    pltData->getYAxis()->set_minTicks(5);
    pltResiduals->getYAxis()->set_minTicks(5);

    //qDebug()<<"   b "<<t.elapsed()<<" ms";
    t.start();

    int errorStyle=cmbErrorStyle->currentIndex();
    int plotStyle=cmbPlotStyle->currentIndex();
    //int residualStyle=cmbResidualStyle->currentIndex();

    if (data->getCorrelationN()>0) {
        size_t c_tau=ds->addColumn(data->getCorrelationT(), data->getCorrelationN(), "tau");


        //////////////////////////////////////////////////////////////////////////////////
        // Plot average + error markers
        //////////////////////////////////////////////////////////////////////////////////
        size_t c_mean=0;
        QString graphName="";
        size_t c_std=0;
        QString errorName="";
        if (eval->getCurrentIndex()<0) {
            c_mean=ds->addColumn(data->getCorrelationMean(), data->getCorrelationN(), "cmean");
            graphName=tr("\\verb{%1} average").arg(record->getName());
            c_std=ds->addColumn(data->getCorrelationStdDev(), data->getCorrelationN(), "cstddev");
            errorName=tr("stddev");
        } else {
            if (eval->getCurrentIndex()<(int)data->getCorrelationRuns()) {
                c_mean=ds->addColumn(data->getCorrelationRun(eval->getCurrentIndex()), data->getCorrelationN(), QString("run"+QString::number(eval->getCurrentIndex())));
                graphName=tr("\\verb{%1} %2").arg(record->getName()).arg(data->getCorrelationRunName(eval->getCurrentIndex()));
                if (eval->getFitDataWeighting()==QFFCSFitEvaluation::RunErrorWeighting) {
                    c_std=ds->addColumn(data->getCorrelationRunError(eval->getCurrentIndex()), data->getCorrelationN(), "cperpixelerror");
                    errorName=tr("per pixel");
                } else {
                    c_std=ds->addColumn(data->getCorrelationStdDev(), data->getCorrelationN(), "cstddev");
                    errorName=tr("stddev");
                }
            } else {
                c_mean=ds->addColumn(data->getCorrelationMean(), data->getCorrelationN(), "cmean");
                graphName=tr("\\verb{%1} average").arg(record->getName());
                c_std=ds->addColumn(data->getCorrelationStdDev(), data->getCorrelationN(), "cstddev");
                errorName=tr("stddev");
            }
        }
        JKQTPerrorPlotstyle styl=JKQTPnoError;
        switch (errorStyle) {
            case 1: styl=JKQTPerrorLines; break;
            case 2: styl=JKQTPerrorBars; break;
            case 3: styl=JKQTPerrorBarsLines; break;
            case 4: styl=JKQTPerrorPolygons; break;
            case 5: styl=JKQTPerrorBarsPolygons; break;
        }
        //qDebug()<<"   c "<<t.elapsed()<<" ms";
        t.start();

        JKQTPxyLineErrorGraph* g=new JKQTPxyLineErrorGraph(pltData->get_plotter());
        QColor gcolor=pltData->get_plotter()->get_graphColor();
        g->set_color(gcolor);
        g->set_errorColor(gcolor.lighter());
        QColor errfc=g->get_errorColor();
        errfc.setAlphaF(0.5);
        g->set_errorFillColor(errfc);
        g->set_lineWidth(2);
        g->set_symbolSize(8);
        g->set_symbolWidth(1);
        g->set_xColumn(c_tau);
        g->set_yColumn(c_mean);
        g->set_yErrorColumn(c_std);
        g->set_title(graphName);
        g->set_yErrorStyle(styl);
        g->set_xErrorStyle(JKQTPnoError);
        g->set_datarange_start(datacut->get_userMin());
        g->set_datarange_end(datacut->get_userMax());
        // draw lines is default
        if (plotStyle==0) { // draw points
            g->set_drawLine(false);
            g->set_symbol(JKQTPcross);
        } else if (plotStyle==2) {
            g->set_symbol(JKQTPcross);
        }
        pltData->addGraph(g);
        //qDebug()<<"   d "<<t.elapsed()<<" ms";
        t.start();

        updateFitFunctions();
        //qDebug()<<"   e "<<t.elapsed()<<" ms";
        t.start();

        pltData->zoomToFit(true, true);
        pltResiduals->zoomToFit(false, true);

        pltResiduals->setX(pltData->getXMin(), pltData->getXMax());

        pltResidualHistogram->zoomToFit(true, true);
        pltResidualCorrelation->zoomToFit(true, true);
        //qDebug()<<"   f "<<t.elapsed()<<" ms";
        t.start();
    }


    pltResiduals->set_doDrawing(true);
    pltResiduals->set_emitSignals(true);
    pltData->set_doDrawing(true);
    pltData->set_emitSignals(true);
    pltResidualHistogram->set_doDrawing(true);
    pltResidualHistogram->set_emitSignals(true);
    pltResidualCorrelation->set_doDrawing(true);
    pltResidualCorrelation->set_emitSignals(true);
    //qDebug()<<"   g "<<t.elapsed()<<" ms";
    t.start();

    pltResiduals->update_plot();
    //qDebug()<<"   h "<<t.elapsed()<<" ms";
    t.start();
    pltData->update_plot();
    //qDebug()<<"   i "<<t.elapsed()<<" ms";
    t.start();
    pltResidualHistogram->update_plot();
    //qDebug()<<"   j "<<t.elapsed()<<" ms";
    t.start();
    pltResidualCorrelation->update_plot();
    //qDebug()<<"   k "<<t.elapsed()<<" ms";
    t.start();
    //qDebug()<<"  replotData end  runtime = "<<t1.elapsed()<<" ms";

    //setUpdatesEnabled(updEn);
}








/////////////////////////////////////////////////////////////////////
// CALIBRATING FOCAL VOLUME
/////////////////////////////////////////////////////////////////////

void QFFCSFitEvaluationEditor::calibrateFocalVolume() {
    double particles=1;
    double tauD=10;
    double particles_error=0;
    double tauD_error=0;
    double wxy=0, wxy_error=0;
    double gamma=6, gamma_error=1;
    bool has_tauD=false;
    bool has_nparticles=false;
    bool has_gamma=false;

    if (!current) return;
    if (!cmbModel) return;
    QFFCSFitEvaluation* eval=qobject_cast<QFFCSFitEvaluation*>(current);
    QFFitFunction* ffunc=eval->getFitFunction();
    if (!ffunc || !eval) return;

    //read input parameters
    for (int i=0; i<ffunc->paramCount(); i++) {
        QString id=ffunc->getParameterID(i);
        QFFitFunction::ParameterDescription d=ffunc->getDescription(i);
        if (id.toLower()=="n_particle") {
            particles=eval->getFitValue(id);
            particles_error=eval->getFitError(id);
            has_nparticles=true;
        }
        if ((id.toLower()=="1n_particle") && (!has_nparticles)) {
            particles=1.0/eval->getFitValue(id);
            particles_error=fabs(eval->getFitError(id)/(eval->getFitValue(id)*eval->getFitValue(id)));
            has_nparticles=true;
        }
        if ((id.toLower()=="focus_struct_fac") && (!has_gamma)) {
            gamma=eval->getFitValue(id);
            gamma_error=eval->getFitError(id);
            has_gamma=true;
        }
        if ((id.toLower()=="diff_tau1") || (id.toLower()=="diff_tau")) {
            double factor=1;
            if (d.unit=="msec") { factor=1000; }
            if (d.unit=="ms") { factor=1000; }
            if (d.unit=="msecs") { factor=1000; }
            if (d.unit=="milliseconds") { factor=1000; }
            if (d.unit=="sec") { factor=1000000; }
            if (d.unit=="s") { factor=1000000; }
            if (d.unit=="secs") { factor=1000000; }
            if (d.unit=="seconds") { factor=1000000; }
            tauD=factor*eval->getFitValue(id);
            tauD_error=factor*eval->getFitError(id);
            has_tauD=true;
        }
    }

    dlgEstimateFocalVolume* dlg=new dlgEstimateFocalVolume(settings, this);
    dlg->init(particles,  particles_error, has_nparticles, tauD, tauD_error, has_tauD, gamma, gamma_error, has_gamma);

    bool ok= ( dlg->exec() == QDialog::Accepted);
    wxy=dlg->get_wxy();
    wxy_error=dlg->get_wxyerror();

    delete dlg;
    //write back output parameters
    if (ok) {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        for (int i=0; i<ffunc->paramCount(); i++) {
            QString id=ffunc->getParameterID(i);
            QFFitFunction::ParameterDescription d=ffunc->getDescription(i);
            if (id.toLower()=="focus_width") {

                double factor=1;
                if (d.unit=="micron") { factor=1000; }
                if (d.unit=="�m") { factor=1000; }
                if (d.unit=="um") { factor=1000; }
                if (d.unit=="microns") { factor=1000; }
                if (d.unit=="m") { factor=1000000; }
                if (d.unit=="meter") { factor=1000000; }
                if (d.unit=="meters") { factor=1000000; }
                eval->setFitValue(id, wxy/factor);
                eval->setFitError(id, wxy_error/factor);
            }
        }
        displayModel(true);
        replotData();
        QApplication::restoreOverrideCursor();
    }
}
















/////////////////////////////////////////////////////////////////////
// REPORT GENERATION
/////////////////////////////////////////////////////////////////////

void QFFCSFitEvaluationEditor::createReportDoc(QTextDocument* document) {
    // make sure all widgets ahave the right size
    {   int trci=tabResidulas->currentIndex();
        for (int i=0;i<tabResidulas->count(); i++) {
            tabResidulas->setCurrentIndex(i);
        }
        tabResidulas->setCurrentIndex(trci);
    }

    int PicTextFormat=QTextFormat::UserObject + 1;
    QObject *picInterface = new QPictureTextObject;
    document->documentLayout()->registerHandler(PicTextFormat, picInterface);

    if (!current) return;
    if (!cmbModel) return;
    QFRawDataRecord* record=current->getHighlightedRecord();
    QFRDRFCSDataInterface* fcs=qobject_cast<QFRDRFCSDataInterface*>(record);
    QFFCSFitEvaluation* eval=qobject_cast<QFFCSFitEvaluation*>(current);
    if (!eval) return;
    if (!fcs) return;

    QFFitFunction* ffunc=eval->getFitFunction();
    QFFitAlgorithm* algorithm=eval->getFitAlgorithm();
    //int run=eval->getCurrentIndex();
    double* params=eval->allocFillParameters();

    QTextCursor cursor(document);
    QTextCharFormat fText=cursor.charFormat();
    fText.setFontPointSize(8);
    QTextCharFormat fTextSmall=fText;
    fTextSmall.setFontPointSize(0.85*fText.fontPointSize());
    QTextCharFormat fTextBold=fText;
    fTextBold.setFontWeight(QFont::Bold);
    QTextCharFormat fTextBoldSmall=fTextBold;
    fTextBoldSmall.setFontPointSize(0.85*fText.fontPointSize());
    QTextCharFormat fHeading1=fText;
    QTextBlockFormat bfLeft;
    bfLeft.setAlignment(Qt::AlignLeft);
    QTextBlockFormat bfLeftNB=bfLeft;
    bfLeftNB.setNonBreakableLines(true);
    QTextBlockFormat bfRight;
    bfRight.setAlignment(Qt::AlignRight);
    QTextBlockFormat bfRightNB=bfRight;
    bfRightNB.setNonBreakableLines(true);
    QTextBlockFormat bfCenter;
    bfCenter.setAlignment(Qt::AlignHCenter);
    QTextBlockFormat bfCenterNB=bfCenter;
    bfCenterNB.setNonBreakableLines(true);

    fHeading1.setFontPointSize(2*fText.fontPointSize());
    fHeading1.setFontWeight(QFont::Bold);
    cursor.insertText(tr("FCS Fit Report:\n\n"), fHeading1);
    cursor.movePosition(QTextCursor::End);

    QTextTableFormat tableFormat;
    tableFormat.setBorderStyle(QTextFrameFormat::BorderStyle_None);
    tableFormat.setWidth(QTextLength(QTextLength::PercentageLength, 98));
    QTextTable* table = cursor.insertTable(3, 4, tableFormat);
    table->cellAt(0, 0).firstCursorPosition().insertText(tr("file:"), fTextBold);
    table->cellAt(0, 1).firstCursorPosition().insertText(record->getName(), fText);
    table->cellAt(0, 2).firstCursorPosition().insertText(tr("run:"), fTextBold);
    table->cellAt(0, 3).firstCursorPosition().insertText(QString("%1 [%2]").arg((eval->getCurrentIndex()<0)?tr("average"):QString::number(eval->getCurrentIndex())).arg(fcs->getCorrelationRunName(eval->getCurrentIndex())), fText);
    table->cellAt(1, 0).firstCursorPosition().insertText(tr("fit algorithm:"), fTextBold);
    table->cellAt(1, 1).firstCursorPosition().insertText(algorithm->name(), fText);
    table->cellAt(1, 2).firstCursorPosition().insertText(tr("data range:"), fTextBold);
    table->cellAt(1, 3).firstCursorPosition().insertText(tr("%1 ... %2 / %3 ... %4").arg(datacut->get_userMin()).arg(datacut->get_userMax()).arg(datacut->get_min()).arg(datacut->get_max()), fText);
    table->cellAt(2, 0).firstCursorPosition().insertText(tr("fit function:"), fTextBold);
    table->cellAt(2, 1).firstCursorPosition().insertText(ffunc->name(), fText);
    table->cellAt(2, 2).firstCursorPosition().insertText(tr("data weighting:"), fTextBold);
    table->cellAt(2, 3).firstCursorPosition().insertText(cmbWeights->currentText(), fText);
    cursor.movePosition(QTextCursor::End);

    cursor.insertBlock(); cursor.insertBlock();
    cursor.insertText(tr("Plots:\n"), fTextBold);
    QTextTableFormat tableFormat1;
    tableFormat1.setBorderStyle(QTextFrameFormat::BorderStyle_None);
    tableFormat1.setWidth(QTextLength(QTextLength::PercentageLength, 98));
    QVector<QTextLength> constraints;
    constraints << QTextLength(QTextLength::PercentageLength, 66);
    constraints << QTextLength(QTextLength::PercentageLength, 34);
    tableFormat1.setColumnWidthConstraints(constraints);
    table = cursor.insertTable(2, 2, tableFormat1);
    table->mergeCells(0,0,2,1);
    {
        QTextCursor tabCursor=table->cellAt(0, 0).firstCursorPosition();
        QPicture pic;
        QPainter* painter=new QPainter(&pic);
        pltData->get_plotter()->draw(*painter, QRect(0,0,pltData->width(),pltData->width()));
        delete painter;
        double scale=document->textWidth()*0.62/pic.boundingRect().width();
        if (scale<=0) scale=1;
        tabCursor.insertText(tr("correlation data, model and residuals:\n"), fTextBoldSmall);
        insertQPicture(tabCursor, PicTextFormat, pic, QSizeF(pic.boundingRect().width(), pic.boundingRect().height())*scale);
    }
    QApplication::processEvents();

    {
        QTextCursor tabCursor=table->cellAt(0, 1).firstCursorPosition();
        QPicture pic;
        QPainter* painter=new QPainter(&pic);
        pltResidualHistogram->get_plotter()->draw(*painter, QRect(0,0,pltResidualHistogram->width(),pltResidualHistogram->width()));
        delete painter;
        double scale=document->textWidth()*0.3/pic.boundingRect().width();
        if (scale<=0) scale=1;
        tabCursor.insertText(tr("residual histogram:\n"), fTextBoldSmall);
        insertQPicture(tabCursor, PicTextFormat, pic, QSizeF(pic.boundingRect().width(), pic.boundingRect().height())*scale);
    }
    QApplication::processEvents();
    {
        QTextCursor tabCursor=table->cellAt(1, 1).firstCursorPosition();
        QPicture pic;
        QPainter* painter=new QPainter(&pic);
        pltResidualCorrelation->get_plotter()->draw(*painter, QRect(0,0,pltResidualCorrelation->width(),pltResidualCorrelation->width()));
        delete painter;
        double scale=document->textWidth()*0.3/pic.boundingRect().width();
        if (scale<=0) scale=1;
        tabCursor.insertText(tr("residual correlations:\n"), fTextBoldSmall);
        insertQPicture(tabCursor, PicTextFormat, pic, QSizeF(pic.boundingRect().width(), pic.boundingRect().height())*scale);
    }
    cursor.movePosition(QTextCursor::End);
    QApplication::processEvents();

    int fitParamCount=0;
    for (int i=0; i<ffunc->paramCount(); i++) {
        if (ffunc->isParameterVisible(i, params)) fitParamCount++;
    }
    QApplication::processEvents();

    cursor.insertBlock(); cursor.insertBlock();
    if (eval->hasFit()) cursor.insertText(tr("Model Parameters (fit results):"), fTextBold);
    else cursor.insertText(tr("Model Parameters (initial values):"), fTextBold);
    QTextTableFormat tableFormat2;
    tableFormat2.setBorderStyle(QTextFrameFormat::BorderStyle_None);
    tableFormat2.setWidth(QTextLength(QTextLength::PercentageLength, 98));
    table = cursor.insertTable(ceil((double)fitParamCount/2.0)+1, 12, tableFormat2);
    QTextCursor tableCursor;
    QApplication::processEvents();

    tableCursor=table->cellAt(0, 0).firstCursorPosition();
    tableCursor.setBlockFormat(bfRight);
    tableCursor.insertText(tr("Parameter"), fTextBoldSmall);
    tableCursor=table->cellAt(0, 2).firstCursorPosition();
    tableCursor.setBlockFormat(bfRight);
    tableCursor.insertText(tr("Value"), fTextBoldSmall);
    if (algorithm->get_supportsBoxConstraints()) table->cellAt(0, 5).firstCursorPosition().insertText(tr("Range"), fTextBoldSmall);
    QApplication::processEvents();

    tableCursor=table->cellAt(0, 6).firstCursorPosition();
    tableCursor.setBlockFormat(bfRight);
    tableCursor.insertText(tr("Parameter"), fTextBoldSmall);
    tableCursor=table->cellAt(0, 8).firstCursorPosition();
    tableCursor.setBlockFormat(bfRight);
    tableCursor.insertText(tr("Value"), fTextBoldSmall);
    if (algorithm->get_supportsBoxConstraints()) table->cellAt(0, 11).firstCursorPosition().insertText(tr("Range"), fTextBoldSmall);
    QApplication::processEvents();


    int rowStart=1;
    int colStart=0;
    for (int i=0; i<ffunc->paramCount(); i++) {
        QApplication::processEvents();
        QString id=ffunc->getParameterID(i);
        double error=roundError(eval->getFitError(id),2);
        double value=roundWithError(eval->getFitValue(id), error, 2);
        QString value_string=floattohtmlstr(value, 5, true).c_str();
        bool fix=eval->getFitFix(id);
        QFFitFunction::ParameterDescription d=ffunc->getDescription(id);
        QString range=QString("%1...%2").arg(QString(floattohtmlstr(d.minValue, 5, true).c_str())).arg(QString(floattohtmlstr(d.maxValue, 5, true).c_str()));
        if ((d.type==QFFitFunction::IntCombo)&&((int)value>=0)&&((int)value<d.comboItems.size())) {
            value_string="<i>"+d.comboItems[(int)value]+"</i>";
        }
        if (ffunc->isParameterVisible(i, params)) {
            QString err="";
            if (d.displayError!=QFFitFunction::NoError) {
                err=QString("&plusmn;&nbsp;%1").arg(QString(floattohtmlstr(error, 5, true).c_str()));
            }

            tableCursor=table->cellAt(rowStart, colStart).firstCursorPosition();
            tableCursor.setBlockFormat(bfRightNB);
            tableCursor.setBlockCharFormat(fTextSmall);
            tableCursor.insertFragment(QTextDocumentFragment::fromHtml(d.label));
            tableCursor.insertText(" = ", fTextSmall);

            tableCursor=table->cellAt(rowStart, colStart+1).firstCursorPosition();
            tableCursor.setBlockFormat(bfLeftNB);
            tableCursor.setBlockCharFormat(fTextSmall);
            if (d.fit) {
                tableCursor.insertText(tr("F"), fTextSmall);
            }
            if (!d.userEditable) {
                tableCursor.insertText(tr("C"), fTextSmall);
            }
            if (fix) {
                tableCursor.insertText(tr("X"), fTextSmall);
            }


            tableCursor=table->cellAt(rowStart, colStart+2).firstCursorPosition();
            tableCursor.setBlockFormat(bfRightNB);
            tableCursor.setBlockCharFormat(fTextSmall);
            tableCursor.insertFragment(QTextDocumentFragment::fromHtml(QString("<nobr>%1</nobr>").arg(value_string)));

            tableCursor=table->cellAt(rowStart, colStart+3).firstCursorPosition();
            tableCursor.setBlockFormat(bfLeftNB);
            tableCursor.setBlockCharFormat(fTextSmall);
            tableCursor.insertFragment(QTextDocumentFragment::fromHtml(QString("<nobr>%1</nobr>").arg(err)));

            tableCursor=table->cellAt(rowStart, colStart+4).firstCursorPosition();
            tableCursor.setBlockFormat(bfLeftNB);
            tableCursor.setBlockCharFormat(fTextSmall);
            tableCursor.insertFragment(QTextDocumentFragment::fromHtml(QString("<nobr>%1</nobr>").arg(d.unitLabel)));

            if (algorithm->get_supportsBoxConstraints()) {
                tableCursor=table->cellAt(rowStart, colStart+5).firstCursorPosition();
                tableCursor.setBlockFormat(bfLeftNB);
                tableCursor.setBlockCharFormat(fTextSmall);
                tableCursor.insertFragment(QTextDocumentFragment::fromHtml(QString("<nobr>%1</nobr>").arg(range)));
            }
            rowStart++;
        };
        if (rowStart>=table->rows()) {
            rowStart=1;
            colStart+=6;
        }
    }
    cursor.movePosition(QTextCursor::End);
    cursor.insertBlock();
    cursor.setBlockFormat(bfCenterNB);
    cursor.setBlockCharFormat(fTextSmall);
    cursor.insertFragment(QTextDocumentFragment::fromHtml(tr("<i><u>legend:</u> <b>F</b>: fit parameter, <b>X</b>: fixed parameter, <b>C</b>: calculated parameter</i>")));
    QApplication::processEvents();

    cursor.setBlockFormat(bfLeft);
    cursor.movePosition(QTextCursor::End);
    cursor.insertBlock(); cursor.insertBlock();
    QString htmlBot=fitStatisticsReport;
    htmlBot.replace("width=\"95%\"", "");
    cursor.insertFragment(QTextDocumentFragment::fromHtml(htmlBot));

}
