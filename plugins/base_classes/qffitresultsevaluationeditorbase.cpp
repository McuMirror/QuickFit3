#include "qffitresultsevaluationeditorbase.h"
#include "qffitresultsevaluation.h"
#include "qffitresultsbyindexevaluation.h"
#include "qmodernprogresswidget.h"


/////////////////////////////////////////////////////////////////////
// CONSTRUCTOR/DESTRUCTOR/INITIALISATION
/////////////////////////////////////////////////////////////////////

QFFitResultsEvaluationEditorBase::QFFitResultsEvaluationEditorBase(QString iniPrefix, QFPluginServices* services, QWidget* parent) :
    QFEvaluationEditor(services, parent)
{
    m_iniPrefix=iniPrefix;
    m_currentFPSSaveDir="";
    m_currentSaveDirectory="";
}


QFFitResultsEvaluationEditorBase::~QFFitResultsEvaluationEditorBase() {

}








/////////////////////////////////////////////////////////////////////
// READING & WRITING PROPERTIES
/////////////////////////////////////////////////////////////////////

void QFFitResultsEvaluationEditorBase::setIniPrefix(QString p) {
    m_iniPrefix=p;
}

QString QFFitResultsEvaluationEditorBase::iniPrefix() const {
    return m_iniPrefix;
}

QString QFFitResultsEvaluationEditorBase::currentFPSSaveDir() const {
    return m_currentFPSSaveDir;
}

void QFFitResultsEvaluationEditorBase::setCurrentFPSSaveDir(QString d) {
    m_currentFPSSaveDir=d;
}

QString QFFitResultsEvaluationEditorBase::currentSaveDirectory() const {
    return m_currentSaveDirectory;
}

void QFFitResultsEvaluationEditorBase::setCurrentSaveDirectory(QString d) {
    m_currentSaveDirectory=d;
}












/////////////////////////////////////////////////////////////////////
// SETTINGS STORAGE
/////////////////////////////////////////////////////////////////////

void QFFitResultsEvaluationEditorBase::readSettings() {
    m_currentFPSSaveDir=settings->getQSettings()->value(m_iniPrefix+"lastFPSDirectory", m_currentFPSSaveDir).toString();
    m_currentSaveDirectory=settings->getQSettings()->value(m_iniPrefix+"lastSaveDirectory", m_currentSaveDirectory).toString();
}

void QFFitResultsEvaluationEditorBase::writeSettings() {
    settings->getQSettings()->setValue(m_iniPrefix+"lastFPSDirectory", m_currentFPSSaveDir);
    settings->getQSettings()->setValue(m_iniPrefix+"lastSaveDirectory", m_currentSaveDirectory);
}














/////////////////////////////////////////////////////////////////////
// READING & WRITING (CURRENT) FIT RESULTS TO INI
/////////////////////////////////////////////////////////////////////

void QFFitResultsEvaluationEditorBase::saveCurrentFitResults() {
    if (!current) return;
    QFFitResultsEvaluation* eval=qobject_cast<QFFitResultsEvaluation*>(current);
    QFFitFunction* ffunc=eval->getFitFunction();
    if (!ffunc || !eval) return;


    QString filter= tr("Fit Parameter Set (*.fps)");
    QString selectedFilter=filter;
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Fit Parameter Set as ..."), m_currentFPSSaveDir, filter, &selectedFilter);
    if ((!fileName.isEmpty())&&(!fileName.isNull())) {
        m_currentFPSSaveDir=QFileInfo(fileName).absolutePath();
        bool ok=true;
        if (QFile::exists(fileName)) {
            int ret = QMessageBox::question(this, tr("Save Fit Parameter Set as  ..."),
                            tr("A Configuration with the name '%1' already exists.\n"
                               "Do you want to overwrite?").arg(fileName),
                            QMessageBox::Yes | QMessageBox::No,  QMessageBox::No);
            if (ret==QMessageBox::No) ok=false;
        }
        if (ok) {
            QSettings settings(fileName, QSettings::IniFormat);
            settings.setValue("fit_function/id", eval->getFitFunction()->id());

            QStringList pids=ffunc->getParameterIDs();
            double* fullParams=eval->allocFillParameters();
            double* errors=eval->allocFillParameterErrors();
            bool* paramsFix=eval->allocFillFix();
            double* paramsMin=eval->allocFillParametersMin();
            double* paramsMax=eval->allocFillParametersMax();
            //ffunc->calcParameter(fullParams, errors);
            for (int i=0; i<pids.size(); i++) {
                QString id=pids[i];
                int num=ffunc->getParameterNum(id);
                if (!id.isEmpty()) {
                    bool visible=ffunc->isParameterVisible(num, fullParams);
                    QFFitFunction::ParameterDescription d=ffunc->getDescription(id);
                    if (!d.userEditable) visible=false;

                    if (visible) {
                        settings.setValue("fit_params/"+id+"/value", eval->getFitValue(id));
                        if (d.displayError==QFFitFunction::EditError) settings.setValue("fit_params/"+id+"/error", eval->getFitError(id));
                        if (d.fit) settings.setValue("fit_params/"+id+"/fix", eval->getFitFix(id));
                        if (d.userRangeEditable) {
                           settings.setValue("fit_params/"+id+"/min", eval->getFitMin(id));
                           settings.setValue("fit_params/"+id+"/max", eval->getFitMax(id));
                        }
                    }
                }
            }
            free(fullParams);
            free(errors);
            free(paramsFix);
            free(paramsMin);
            free(paramsMax);
        }
    }
}

void QFFitResultsEvaluationEditorBase::loadCurrentFitResults() {
    if (!current) return;
    QFFitResultsEvaluation* eval=qobject_cast<QFFitResultsEvaluation*>(current);
    QFFitFunction* ffunc=eval->getFitFunction();
    if (!ffunc || !eval) return;

    QString filter= tr("Fit Parameter Set (*.fps)");
    QString selectedFilter=filter;
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load Fit Parameter Set ..."), m_currentFPSSaveDir, filter, &selectedFilter);
    if ((!fileName.isEmpty())&&(!fileName.isNull())) {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        eval->set_doEmitPropertiesChanged(false);
        m_currentFPSSaveDir=QFileInfo(fileName).absolutePath();
        QSettings settings(fileName, QSettings::IniFormat);
        QString ffuncname=settings.value("fit_function/id", eval->getFitFunction()->id()).toString();
        eval->setFitFunction(ffuncname);
        settings.beginGroup("fit_params");
        QStringList keys = settings.allKeys();
        for (int i=0; i<keys.size(); i++) {
            if (keys[i].endsWith("/error")) {
                QString paramname=keys[i].left(keys[i].length()-6);
                eval->setFitError(paramname, settings.value(keys[i]).toDouble());
            } else if (keys[i].endsWith("/value")) {
                QString paramname=keys[i].left(keys[i].length()-6);
                eval->setFitValue(paramname, settings.value(keys[i]).toDouble());
            } else if (keys[i].endsWith("/fix")) {
                QString paramname=keys[i].left(keys[i].length()-4);
                eval->setFitFix(paramname, settings.value(keys[i]).toBool());
            } else if (keys[i].endsWith("/min")) {
                QString paramname=keys[i].left(keys[i].length()-4);
                eval->setFitMin(paramname, settings.value(keys[i]).toDouble());
            } else if (keys[i].endsWith("/max")) {
                QString paramname=keys[i].left(keys[i].length()-4);
                eval->setFitMax(paramname, settings.value(keys[i]).toDouble());
            }
        }
        eval->set_doEmitPropertiesChanged(true);
        eval->emitPropertiesChanged();
        displayModel(true);
        replotData();
        QApplication::restoreOverrideCursor();
    }
}













/////////////////////////////////////////////////////////////////////
// REPORT GENERATION
/////////////////////////////////////////////////////////////////////


void QFFitResultsEvaluationEditorBase::createReportDoc(QTextDocument* document) {

}


void QFFitResultsEvaluationEditorBase::saveReport() {
    QString fn = QFileDialog::getSaveFileName(this, tr("Save Report"),
                                m_currentSaveDirectory,
                                tr("PDF File (*.pdf);;PostScript File (*.ps)"));

    if (!fn.isEmpty()) {
        m_currentSaveDirectory=QFileInfo(fn).absolutePath();
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        QModernProgressDialog progress(tr("Exporting ..."), "", NULL);
        progress.setWindowModality(Qt::WindowModal);
        progress.setHasCancel(false);
        progress.setLabelText(tr("saving report <br> to '%1' ...").arg(fn));
        progress.open();

        QFileInfo fi(fn);
        QPrinter* printer=new QPrinter();//QPrinter::HighResolution);
        printer->setPaperSize(QPrinter::A4);
        printer->setPageMargins(15,15,15,15,QPrinter::Millimeter);
        printer->setOrientation(QPrinter::Portrait);
        printer->setOutputFormat(QPrinter::PdfFormat);
        if (fi.suffix().toLower()=="ps") printer->setOutputFormat(QPrinter::PostScriptFormat);
        printer->setOutputFileName(fn);
        QTextDocument* doc=new QTextDocument();
        doc->setTextWidth(printer->pageRect().size().width());
        createReportDoc(doc);
        doc->print(printer);
        //qDebug()<<doc->toHtml();
        delete doc;
        delete printer;
        progress.accept();
        QApplication::restoreOverrideCursor();
    }
}

void QFFitResultsEvaluationEditorBase::printReport() {
    QPrinter* p=new QPrinter();//QPrinter::HighResolution);

    p->setPageMargins(15,15,15,15,QPrinter::Millimeter);
    p->setOrientation(QPrinter::Portrait);
    QPrintDialog *dialog = new QPrintDialog(p, this);
    dialog->setWindowTitle(tr("Print Report"));
    if (dialog->exec() != QDialog::Accepted) {
        delete p;
        return;
    }

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QModernProgressDialog progress(tr("Printing ..."), "", NULL);
    progress.setWindowModality(Qt::WindowModal);
    progress.setHasCancel(false);
    progress.setLabelText(tr("printing report ..."));
    progress.open();
    QTextDocument* doc=new QTextDocument();
    doc->setTextWidth(p->pageRect().size().width());
    createReportDoc(doc);
    doc->print(p);
    delete p;
    progress.accept();
    QApplication::restoreOverrideCursor();
}