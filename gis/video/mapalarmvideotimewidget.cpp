#include "mapalarmvideotimewidget.h"
#include "ui_mapalarmvideotimewidget.h"
#include <QCalendarWidget>
#include "apptranslationhelper.h"

MapAlarmVideoTimeWidget::MapAlarmVideoTimeWidget(QWidget *parent) :
    SNDialog(parent),
    ui(new Ui::MapAlarmVideoTimeWidget)
{
    ui->setupUi(this);
    initForm();
}

MapAlarmVideoTimeWidget::~MapAlarmVideoTimeWidget()
{
    delete ui;
}

void MapAlarmVideoTimeWidget::initForm()
{
    //日期选择控件设置
    ui->dateTimeEditStart->setDisplayFormat("yyyy-MM-dd HH:mm:ss");
    ui->dateTimeEditEnd->setDisplayFormat("yyyy-MM-dd HH:mm:ss");
    ui->dateTimeEditStart->setCalendarPopup(true);
    ui->dateTimeEditEnd->setCalendarPopup(true);
    ui->dateTimeEditStart->setDateTime(QDateTime::currentDateTime().addDays(-1));
    QTextCharFormat format = ui->dateTimeEditStart->calendarWidget()->weekdayTextFormat(Qt::Monday);
    format.setForeground(QBrush(QColor("#CCCCCC")));
    ui->dateTimeEditStart->calendarWidget()->setWeekdayTextFormat(Qt::Sunday,format);
    ui->dateTimeEditStart->calendarWidget()->setWeekdayTextFormat(Qt::Saturday,format);
    ui->dateTimeEditEnd->calendarWidget()->setWeekdayTextFormat(Qt::Sunday,format);
    ui->dateTimeEditEnd->calendarWidget()->setWeekdayTextFormat(Qt::Saturday,format);
    ui->dateTimeEditEnd->setDateTime(QDateTime::currentDateTime());
    connect(ui->dateTimeEditStart, SIGNAL(dateTimeChanged(QDateTime)), this, SLOT(slotAdjustDatetime(QDateTime)));
    connect(ui->dateTimeEditEnd, SIGNAL(dateTimeChanged(QDateTime)), this, SLOT(slotAdjustDatetime(QDateTime)));
    ui->dateTimeEditStart->setMaximumDateTime(QDateTime::currentDateTime().addDays(1));
    ui->dateTimeEditEnd->setMaximumDateTime(QDateTime::currentDateTime().addDays(1));

    AppStyleHelper::updateWidgetStyle(ui->pushButtonCancel, AppStyleHelper::Style_PushButtonDefault);
    AppStyleHelper::updateWidgetStyle(ui->pushButtonOk, AppStyleHelper::Style_PushButtonDefault);

    retranslateUi();
}

void MapAlarmVideoTimeWidget::retranslateUi()
{
    translateCalendar();
    ui->labelStartTime->setText(QObject::tr("TK_StartTime"));
    ui->labelEndTime->setText(QObject::tr("TK_EndTime"));
    ui->pushButtonCancel->setText(QObject::tr("TK_Cancel"));
    ui->pushButtonOk->setText(QObject::tr("TK_Confirm"));
    ui->labelTitle->setText(QObject::tr("TK_TimeSelect"));
}

void MapAlarmVideoTimeWidget::openDialog()
{
    showSNDialog();
}

bool MapAlarmVideoTimeWidget::openDialog(QDateTime start,QDateTime end)
{
    if(!start.isValid())
    {
        return false;
    }
    if(!end.isValid())
    {
        return false;
    }
    ui->dateTimeEditStart->setDateTime(start);
    ui->dateTimeEditEnd->setDateTime(end);
    showSNDialog();
    return true;
}

bool MapAlarmVideoTimeWidget::setTimeParam(QDateTime start,QDateTime end)
{
    if(!start.isValid())
    {
        return false;
    }
    if(!end.isValid())
    {
        return false;
    }
    ui->dateTimeEditStart->setDateTime(start);
    ui->dateTimeEditEnd->setDateTime(end);
    return true;
}

void MapAlarmVideoTimeWidget::getTimeParam(QDateTime &start,QDateTime &end)
{
    start = ui->dateTimeEditStart->dateTime();
    end = ui->dateTimeEditEnd->dateTime();
}

void MapAlarmVideoTimeWidget::on_pushButtonClose_clicked()
{
    close();
}

void MapAlarmVideoTimeWidget::on_pushButtonOk_clicked()
{
    emit timeSelected(ui->dateTimeEditStart->dateTime(),ui->dateTimeEditEnd->dateTime());
    close();
}

void MapAlarmVideoTimeWidget::on_pushButtonCancel_clicked()
{
    close();
}

void MapAlarmVideoTimeWidget::slotAdjustDatetime(const QDateTime &datetime)
{
    QDateTimeEdit *timeEdit  = qobject_cast<QDateTimeEdit *>(sender());

    ui->dateTimeEditStart->setMaximumDateTime(QDateTime::currentDateTime().addDays(1));
    ui->dateTimeEditEnd->setMaximumDateTime(QDateTime::currentDateTime().addDays(1));
    const int  minday = 28 ; //  一个月最低天数
    if(ui->dateTimeEditStart == timeEdit)
    {

        int secs = datetime.daysTo(ui->dateTimeEditEnd->dateTime());

        if(secs < 0 || secs > minday)
        {
            ui->dateTimeEditEnd->setDateTime(datetime.addMonths(1));
        }

    }
    else if(ui->dateTimeEditEnd == timeEdit)
    {
        int secs = ui->dateTimeEditStart->dateTime().daysTo(datetime);
        if(secs < 0 || secs > minday)
        {
            ui->dateTimeEditStart->setDateTime(datetime.addMonths(-1));
        }
    }
}

void MapAlarmVideoTimeWidget::translateCalendar()
{
    QLocale::Language language = g_TranslationHelper->getCurrentLanguage();
    ui->dateTimeEditEnd->setLocale(language);
    ui->dateTimeEditEnd->calendarWidget()->setLocale(language);

    ui->dateTimeEditStart->setLocale(language);
    ui->dateTimeEditStart->calendarWidget()->setLocale(language);
}
