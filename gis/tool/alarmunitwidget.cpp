#include "alarmunitwidget.h"
#include "ui_alarmunitwidget.h"
#include "sppclientalarmmanager.h"
#include "apptranslationhelper.h"

//GIS地图功能比较复杂，在对本模块框架代码完全熟悉之前，不要擅自修改，否则会留下潜在风险 zuoshul20190715

AlarmUnitWidget::AlarmUnitWidget(QWidget *parent) :
    BaseWidget(parent),
    ui(new Ui::AlarmUnitWidget),
    m_pObjAlarmVideoWidget(NULL)
{
    ui->setupUi(this);
    initForm();
}

AlarmUnitWidget::~AlarmUnitWidget()
{
    delete m_pObjAlarmVideoWidget;
    m_pObjAlarmVideoWidget = NULL;
    delete ui;
}

void AlarmUnitWidget::initForm()
{
    if(NULL == m_pObjAlarmVideoWidget)
    {
        //此处加this会有异常
        m_pObjAlarmVideoWidget = new FormAlarmVideoWidget();
        if(NULL != m_pObjAlarmVideoWidget)
        {
            m_pObjAlarmVideoWidget->setVisible(false);
        }
    }
    AppStyleHelper::updateWidgetStyle(ui->pushButtonPlay, AppStyleHelper::Style_PushButtonLight);
    retranslateUi();
}

void AlarmUnitWidget::retranslateUi()
{
    ui->retranslateUi(this);
}

void AlarmUnitWidget::doAfterWidgetShow()
{

}

void AlarmUnitWidget::setParam(Param pa)
{
    m_param = pa;

    QString typeName;
    InviewAlarmTypeList list;
    g_SPPClientAlarmManager->getAlarmTypeList(list);
    for(int i = 0; i < list.size();i++)
    {
        InviewAlarmType type = list.at(i);
        if(type.get_alarm_type_code() == pa.type)
        {
            typeName = g_TranslationHelper->getStringAfterTranslation(type.get_alarm_type_name().c_str());
        }
    }
    ui->labelDevName->setText(pa.devName);
    ui->labelTime->setText(pa.time);
    ui->labelNumber->setText(QString::number(pa.number));
    ui->labelAlarmType->setText(typeName);
}

AlarmUnitWidget::Param AlarmUnitWidget::getParam()
{
    return m_param;
}

void AlarmUnitWidget::on_pushButtonPlay_clicked()
{
    if(m_pObjAlarmVideoWidget)
    {
        if (m_pObjAlarmVideoWidget->initForm(OperatorTypePlayreal, m_param.uid, m_param.alarmTime, m_param.devName))
        {
            m_pObjAlarmVideoWidget->setVisible(true);
        }
        else
        {
            setTips(QObject::tr("TK_DeviceNotExist"), SNMessageBoxFlowDownWidget::MessageWarring);
        }
    }
}
