/*
    Copyright (c) 2008-2014 Jan W. Krieger, German Cancer Research Center

    last modification: $LastChangedDate$  (revision $Rev$)

    This file is part of QuickFit 3 (http://www.dkfz.de/Macromol/quickfit).

    This software is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "qffitresultsbyindexevaluationeditorbase.h"

QFFitResultsByIndexEvaluationEditorBase::QFFitResultsByIndexEvaluationEditorBase(QString iniPrefix, QFEvaluationPropertyEditor *propEditor, QFPluginServices *services, QWidget *parent) :
    QFFitResultsEvaluationEditorBase(iniPrefix, services, propEditor, parent)
{
}














/////////////////////////////////////////////////////////////////////
// SETTING PROPERTIES IN OTHER FILES&RUNS&FILES^RUNS
/////////////////////////////////////////////////////////////////////

void QFFitResultsByIndexEvaluationEditorBase::copyUserMinToAll(int userMin) {
    if (!current) return;
    QFFitResultsByIndexEvaluation* data=qobject_cast<QFFitResultsByIndexEvaluation*>(current);
    if (!data) return;
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QList<QFRawDataRecord*> recs=current->getProject()->getRawDataList();

    for (int i=0; i<recs.size(); i++) {
        if (current->isFilteredAndApplicable(recs[i])) {
            QFRawDataRecord* rdr=recs[i];
            rdr->disableEmitPropertiesChanged();
            for (int r=data->getIndexMin(rdr); r<=data->getIndexMax(rdr); r++) {
                const QString resultID=data->getEvaluationResultID(r, rdr);
                if (!((rdr==current->getHighlightedRecord())&&(r==data->getCurrentIndex()))) {
                    rdr->setQFProperty(resultID+"_datacut_min", userMin, false, false);
                }
            }
            rdr->enableEmitPropertiesChanged(true);
        }
    }
    QApplication::restoreOverrideCursor();
}

void QFFitResultsByIndexEvaluationEditorBase::copyUserMaxToAll(int userMax) {
    if (!current) return;
    QFFitResultsByIndexEvaluation* data=qobject_cast<QFFitResultsByIndexEvaluation*>(current);
    if (!data) return;
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QList<QFRawDataRecord*> recs=current->getProject()->getRawDataList();

    for (int i=0; i<recs.size(); i++) {
        if (current->isFilteredAndApplicable(recs[i])) {
            QFRawDataRecord* rdr=recs[i];
            rdr->disableEmitPropertiesChanged();
            for (int r=data->getIndexMin(rdr); r<=data->getIndexMax(rdr); r++) {
                const QString resultID=data->getEvaluationResultID(r, rdr);
                if (!((rdr==current->getHighlightedRecord())&&(r==data->getCurrentIndex()))) {
                    rdr->setQFProperty(resultID+"_datacut_max", userMax, false, false);
                }
            }
            rdr->enableEmitPropertiesChanged(true);
        }
    }
    QApplication::restoreOverrideCursor();
}


void QFFitResultsByIndexEvaluationEditorBase::copyUserMinMaxToAll(int userMin, int userMax) {
    if (!current) return;
    QFFitResultsByIndexEvaluation* data=qobject_cast<QFFitResultsByIndexEvaluation*>(current);
    if (!data) return;
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QList<QFRawDataRecord*> recs=current->getProject()->getRawDataList();

    for (int i=0; i<recs.size(); i++) {
        if (current->isFilteredAndApplicable(recs[i])) {
            QFRawDataRecord* rdr=recs[i];
            rdr->disableEmitPropertiesChanged();
            for (int r=data->getIndexMin(rdr); r<=data->getIndexMax(rdr); r++) {
                const QString resultID=data->getEvaluationResultID(r, rdr);
                if (!((rdr==current->getHighlightedRecord())&&(r==data->getCurrentIndex()))) {
                    rdr->setQFProperty(resultID+"_datacut_min", userMin, false, false);
                    rdr->setQFProperty(resultID+"_datacut_max", userMax, false, false);
                }
            }
            rdr->enableEmitPropertiesChanged(true);
        }
    }
    QApplication::restoreOverrideCursor();
}

void QFFitResultsByIndexEvaluationEditorBase::copyUserMinToAllRuns(int userMin) {
    if (!current) return;
    QFFitResultsByIndexEvaluation* data=qobject_cast<QFFitResultsByIndexEvaluation*>(current);
    if (!data) return;
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QFRawDataRecord* rdr=data->getHighlightedRecord();
    rdr->disableEmitPropertiesChanged();
    for (int r=data->getIndexMin(rdr); r<=data->getIndexMax(rdr); r++) {
        const QString resultID=data->getEvaluationResultID(r, rdr);
        if (!((rdr==current->getHighlightedRecord())&&(r==data->getCurrentIndex()))) {
            rdr->setQFProperty(resultID+"_datacut_min", userMin, false, false);
        }
    }
    rdr->enableEmitPropertiesChanged(true);
    QApplication::restoreOverrideCursor();
}



void QFFitResultsByIndexEvaluationEditorBase::copyUserMaxToAllRuns(int userMax) {
    if (!current) return;
    QFFitResultsByIndexEvaluation* data=qobject_cast<QFFitResultsByIndexEvaluation*>(current);
    if (!data) return;
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QFRawDataRecord* rdr=data->getHighlightedRecord();
    rdr->disableEmitPropertiesChanged();
    for (int r=data->getIndexMin(rdr); r<=data->getIndexMax(rdr); r++) {
        const QString resultID=data->getEvaluationResultID(r, rdr);
        if (!((rdr==current->getHighlightedRecord())&&(r==data->getCurrentIndex()))) {
            rdr->setQFProperty(resultID+"_datacut_max", userMax, false, false);
        }
    }
    rdr->enableEmitPropertiesChanged(true);
    QApplication::restoreOverrideCursor();
}


void QFFitResultsByIndexEvaluationEditorBase::copyUserMinMaxToAllRuns(int userMin, int userMax) {
    if (!current) return;
    QFFitResultsByIndexEvaluation* data=qobject_cast<QFFitResultsByIndexEvaluation*>(current);
    if (!data) return;
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QFRawDataRecord* rdr=data->getHighlightedRecord();
    rdr->disableEmitPropertiesChanged();
    for (int r=data->getIndexMin(rdr); r<=data->getIndexMax(rdr); r++) {
        const QString resultID=data->getEvaluationResultID(r, rdr);
        if (!((rdr==current->getHighlightedRecord())&&(r==data->getCurrentIndex()))) {
            rdr->setQFProperty(resultID+"_datacut_min", userMin, false, false);
            rdr->setQFProperty(resultID+"_datacut_max", userMax, false, false);
        }
    }
    rdr->enableEmitPropertiesChanged(true);
    QApplication::restoreOverrideCursor();
}




int QFFitResultsByIndexEvaluationEditorBase::getUserMin(QFRawDataRecord* rec, int index, int defaultMin) {
    QFFitResultsByIndexEvaluation* data=qobject_cast<QFFitResultsByIndexEvaluation*>(current);
    if (!data) return defaultMin;
    const QString resultID=data->getEvaluationResultID(index, rec);

    // WORKROUND FOR OLD PROPERTY NAMES
    int defaultM=rec->getProperty(QString(resultID+"_datacut_min").replace(QString("_")+data->getFitFunction(rec)->id()+QString("_run"), "_r").replace(data->getType()+"_", data->getType()), defaultMin).toInt();

    return rec->getProperty(resultID+"_datacut_min", defaultM).toInt();
}

int QFFitResultsByIndexEvaluationEditorBase::getUserMax(QFRawDataRecord* rec, int index, int defaultMax) {
    QFFitResultsByIndexEvaluation* data=qobject_cast<QFFitResultsByIndexEvaluation*>(current);
    if (!data) return defaultMax;
    const QString resultID=data->getEvaluationResultID(index, rec);

    // WORKROUND FOR OLD PROPERTY NAMES
    int defaultM=rec->getProperty(QString(resultID+"_datacut_max").replace(QString("_")+data->getFitFunction(rec)->id()+QString("_run"), "_r").replace(data->getType()+"_", data->getType()), defaultMax).toInt();

    return rec->getProperty(resultID+"_datacut_max", defaultM).toInt();
    //return rec->getProperty(resultID+"_datacut_max", defaultMax).toInt();
}


int QFFitResultsByIndexEvaluationEditorBase::getUserMin(QFRawDataRecord* rec) {
    QFFitResultsByIndexEvaluation* data=qobject_cast<QFFitResultsByIndexEvaluation*>(current);
    if (!data) return 0;
    return getUserMin(rec, data->getCurrentIndex());
}

int QFFitResultsByIndexEvaluationEditorBase::getUserMax(QFRawDataRecord* rec) {
    QFFitResultsByIndexEvaluation* data=qobject_cast<QFFitResultsByIndexEvaluation*>(current);
    if (!data) return 0;
    return getUserMax(rec, data->getCurrentIndex());
}
























void QFFitResultsByIndexEvaluationEditorBase::resetCurrent() {
    if (!current) return;
    //QFRawDataRecord* record=current->getHighlightedRecord();
    QFFitResultsByIndexEvaluation* eval=qobject_cast<QFFitResultsByIndexEvaluation*>(current);
    if (!eval) return;
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    eval->resetAllFitResultsCurrent();
    displayModel(false);
    replotData();
    QApplication::restoreOverrideCursor();
}

void QFFitResultsByIndexEvaluationEditorBase::resetAll() {
    if (!current) return;
    QFFitResultsByIndexEvaluation* eval=qobject_cast<QFFitResultsByIndexEvaluation*>(current);
    if (!eval) return;
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    eval->resetAllFitResultsAllFilesAllIndices();
    //bool updEn=updatesEnabled();
    //setUpdatesEnabled(false);
    displayModel(false);
    replotData();
    //setUpdatesEnabled(updEn);
    QApplication::restoreOverrideCursor();
}

void QFFitResultsByIndexEvaluationEditorBase::resetAllRuns() {
    if (!current) return;
    QFFitResultsByIndexEvaluation* eval=qobject_cast<QFFitResultsByIndexEvaluation*>(current);
    if (!eval) return;
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    eval->resetAllFitResultsCurrentFileAllIndices();
    //bool updEn=updatesEnabled();
    //setUpdatesEnabled(false);
    displayModel(false);
    replotData();
    //setUpdatesEnabled(updEn);
    QApplication::restoreOverrideCursor();
}

void QFFitResultsByIndexEvaluationEditorBase::copyToAll() {

    if (!current) return;
    QFFitResultsByIndexEvaluation* eval=qobject_cast<QFFitResultsByIndexEvaluation*>(current);
    if (!eval) return;
    copyToInitial(false, eval->getHighlightedRecord());


    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    double* params=eval->allocFillParameters();

    QList<QPointer<QFRawDataRecord> > recs=eval->getApplicableRecords();

    eval->set_doEmitResultsChanged(false);
    eval->set_doEmitPropertiesChanged(false);


    for (int i=0; i<recs.size(); i++) {
        QFRawDataRecord* record=recs[i];
        if (record) {
            QFFitFunction* ffunc=eval->getFitFunction(record);
            record->disableEmitResultsChanged();
            for (int i=0; i<ffunc->paramCount(); i++) {
                QString id=ffunc->getParameterID(i);
                double value=eval->getFitValue(id);
                double error=eval->getFitError(id);
                bool fix=eval->getFitFix(id);
                if (ffunc->isParameterVisible(i, params)) {
                    for (int run=eval->getIndexMin(record); run<=eval->getIndexMax(record); run++) {
                        if (eval->hasFit(record, run)) {
                            eval->setFitResultValue(record, run, id, value, error);
                            eval->setFitResultFix(record, run, id, fix);
                        }
                    }
                }
            }
            record->enableEmitResultsChanged();
        };
    }

    eval->set_doEmitResultsChanged(true);
    eval->set_doEmitPropertiesChanged(true);
    eval->emitResultsChanged();

    free(params);
    QApplication::restoreOverrideCursor();
}


void QFFitResultsByIndexEvaluationEditorBase::copyToAllRuns() {

    if (!current) return;
    QFFitResultsByIndexEvaluation* eval=qobject_cast<QFFitResultsByIndexEvaluation*>(current);
    if (!eval) return;
    copyToInitial(false, eval->getHighlightedRecord());


    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    double* params=eval->allocFillParameters();

    eval->set_doEmitResultsChanged(false);
    eval->set_doEmitPropertiesChanged(false);

    QFRawDataRecord* record=eval->getHighlightedRecord();
    if (record) {
        QFFitFunction* ffunc=eval->getFitFunction(record);
        record->disableEmitResultsChanged();
        for (int i=0; i<ffunc->paramCount(); i++) {
            QString id=ffunc->getParameterID(i);
            double value=eval->getFitValue(id);
            double error=eval->getFitError(id);
            bool fix=eval->getFitFix(id);
            if (ffunc->isParameterVisible(i, params)) {
                for (int run=eval->getIndexMin(record); run<=eval->getIndexMax(record); run++) {
                    if (eval->hasFit(record, run)) {
                        eval->setFitResultValue(record, run, id, value, error);
                        eval->setFitResultFix(record, run, id, fix);
                    }
                }
            };
        }
        record->enableEmitResultsChanged();
    }

    eval->set_doEmitResultsChanged(true);
    eval->set_doEmitPropertiesChanged(true);
    eval->emitResultsChanged();

    free(params);
    QApplication::restoreOverrideCursor();
}

void QFFitResultsByIndexEvaluationEditorBase::copyToAllCurrentRun() {

    if (!current) return;
    QFFitResultsByIndexEvaluation* eval=qobject_cast<QFFitResultsByIndexEvaluation*>(current);
    if (!eval) return;
    copyToInitial(false, eval->getHighlightedRecord());


    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    double* params=eval->allocFillParameters();

    QList<QPointer<QFRawDataRecord> > recs=eval->getApplicableRecords();

    eval->set_doEmitResultsChanged(false);
    eval->set_doEmitPropertiesChanged(false);

    for (int i=0; i<recs.size(); i++) {
        QFRawDataRecord* record=recs[i];
        if (record) {
            QFFitFunction* ffunc=eval->getFitFunction(record);
            record->disableEmitResultsChanged();
            for (int i=0; i<ffunc->paramCount(); i++) {
                QString id=ffunc->getParameterID(i);
                double value=eval->getFitValue(id);
                double error=eval->getFitError(id);
                bool fix=eval->getFitFix(id);
                if (ffunc->isParameterVisible(i, params)) {

                    int run=eval->getCurrentIndex();

                    if (eval->hasFit(record, run)) {
                        eval->setFitResultValue(record, run, id, value, error);
                        eval->setFitResultFix(record, run, id, fix);
                    }
                }
            }
            record->enableEmitResultsChanged();
        };
    }
    eval->set_doEmitResultsChanged(true);
    eval->set_doEmitPropertiesChanged(true);
    eval->emitResultsChanged();

    free(params);
    QApplication::restoreOverrideCursor();
}

void QFFitResultsByIndexEvaluationEditorBase::setParamAllIndexes(QFRawDataRecord *rec, const QString &parameter, const QVector<double> &data, const QVector<double> &dataerror)
{
    if (!current) return;
    QFFitResultsByIndexEvaluation* eval=qobject_cast<QFFitResultsByIndexEvaluation*>(current);
    if (!eval) return;
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    bool doEmit=rec->isEmitResultsChangedEnabled();
    rec->disableEmitResultsChanged();
    bool thisEmit=eval->get_doEmitResultsChanged();
    eval->set_doEmitResultsChanged(false);
    for (int i=0; i<data.size(); i++) {
        eval->setFitResultValue(rec, i, parameter, data.value(i, 0), dataerror.value(i, 0));
    }
    if (doEmit) rec->enableEmitResultsChanged(true);
    eval->set_doEmitResultsChanged(thisEmit);
    eval->emitResultsChanged(rec);
    displayModel(false);
    replotData();
    QApplication::restoreOverrideCursor();
}

void QFFitResultsByIndexEvaluationEditorBase::setParamAllIndexesInCurrent(const QString &parameter, const QVector<double> &data)
{
    if (!current) return;
    setParamAllIndexes(current->getHighlightedRecord(), parameter, data);
}


void QFFitResultsByIndexEvaluationEditorBase::log_text(QString message)
{
    QFPluginLogTools::log_text(message);
}

void QFFitResultsByIndexEvaluationEditorBase::log_warning(QString message)
{
    QFPluginLogTools::log_warning(message);
}

void QFFitResultsByIndexEvaluationEditorBase::log_error(QString message)
{
    QFPluginLogTools::log_error(message);
}




