#include <CQGradientPaletteIFace.h>
#include <CQGradientPalettePlot.h>
#include <CQRealSpin.h>
#include <CQColorChooser.h>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QStackedWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QDir>
#include <QHeaderView>
#include <QCheckBox>
#include <QItemDelegate>
#include <cassert>

CQGradientPaletteIFace::
CQGradientPaletteIFace(CQGradientPalettePlot *palette) :
 QFrame(0), palette_(palette)
{
  setObjectName("paletteControl");

  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

  QFontMetrics fm(font());

  //---

  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setMargin(2); layout->setSpacing(2);

  CGradientPalette *pal = palette_->gradientPalette();

  //---

  QFrame *colorTypeFrame = createColorTypeCombo("Type", &colorType_);

  colorType_->setType(pal->colorType());

  connect(colorType_, SIGNAL(currentIndexChanged(int)), this, SLOT(colorTypeChanged(int)));

  layout->addWidget(colorTypeFrame);

  //---

  QFrame *colorModelFrame = createColorModelCombo("Color", &colorModel_);

  colorModel_->setModel(pal->colorModel());

  connect(colorModel_, SIGNAL(currentIndexChanged(int)), this, SLOT(colorModelChanged(int)));

  layout->addWidget(colorModelFrame);

  //---

  stack_ = new QStackedWidget;

  stack_->setObjectName("stack");

  //---

  QFrame *modelFrame = new QFrame;

  modelFrame->setObjectName("modelFrame");

  QGridLayout *modelGridLayout = new QGridLayout(modelFrame);
  modelGridLayout->setMargin(2); modelGridLayout->setSpacing(2);

  createModelCombo(modelGridLayout, 0, "Red"  , &redModel_  );
  createModelCombo(modelGridLayout, 1, "Green", &greenModel_);
  createModelCombo(modelGridLayout, 2, "Blue" , &blueModel_ );

  setRedModel  (pal->redModel  ());
  setGreenModel(pal->greenModel());
  setBlueModel (pal->blueModel ());

  connect(redModel_  , SIGNAL(currentIndexChanged(int)), this, SLOT(modelChanged(int)));
  connect(greenModel_, SIGNAL(currentIndexChanged(int)), this, SLOT(modelChanged(int)));
  connect(blueModel_ , SIGNAL(currentIndexChanged(int)), this, SLOT(modelChanged(int)));

  //---

  QGroupBox *negateGroupBox = new QGroupBox("Negate");

  negateGroupBox->setContentsMargins(0, fm.height() + 2, 0, 0);

  negateGroupBox->setObjectName("negate");

  QHBoxLayout *negateGroupLayout = new QHBoxLayout(negateGroupBox);
  negateGroupLayout->setMargin(0); negateGroupLayout->setSpacing(2);

  modelGridLayout->addWidget(negateGroupBox, 3, 0, 1, 2);

  modelRNegativeCheck_ = new QCheckBox("Red");
  modelGNegativeCheck_ = new QCheckBox("Green");
  modelBNegativeCheck_ = new QCheckBox("Blue");

  modelRNegativeCheck_->setObjectName("rnegative");
  modelGNegativeCheck_->setObjectName("gnegative");
  modelBNegativeCheck_->setObjectName("bnegative");

  connect(modelRNegativeCheck_, SIGNAL(stateChanged(int)), this, SLOT(modelRNegativeChecked(int)));
  connect(modelGNegativeCheck_, SIGNAL(stateChanged(int)), this, SLOT(modelGNegativeChecked(int)));
  connect(modelBNegativeCheck_, SIGNAL(stateChanged(int)), this, SLOT(modelBNegativeChecked(int)));

  negateGroupLayout->addWidget(modelRNegativeCheck_);
  negateGroupLayout->addWidget(modelGNegativeCheck_);
  negateGroupLayout->addWidget(modelBNegativeCheck_);
  negateGroupLayout->addStretch(1);

  //---

  QGroupBox *rangeGroupBox = new QGroupBox("Range");

  rangeGroupBox->setContentsMargins(0, fm.height() + 2, 0, 0);

  rangeGroupBox->setObjectName("range");

  QHBoxLayout *rangeGroupLayout = new QHBoxLayout(rangeGroupBox);
  rangeGroupLayout->setMargin(0); rangeGroupLayout->setSpacing(2);

  modelGridLayout->addWidget(rangeGroupBox, 4, 0, 1, 2);

  QGridLayout *rangeGridLayout = new QGridLayout;

  createRealEdit(rangeGridLayout, 0, "Red Min"  , &redMin_  );
  createRealEdit(rangeGridLayout, 1, "Red Max"  , &redMax_  );
  createRealEdit(rangeGridLayout, 2, "Green Min", &greenMin_);
  createRealEdit(rangeGridLayout, 3, "Green Max", &greenMax_);
  createRealEdit(rangeGridLayout, 4, "Blue Min" , &blueMin_ );
  createRealEdit(rangeGridLayout, 5, "Blue Max" , &blueMax_ );

  rangeGroupLayout->addLayout(rangeGridLayout);
  rangeGroupLayout->addStretch(1);

  connect(redMin_  , SIGNAL(valueChanged(double)), this, SLOT(modelRangeValueChanged(double)));
  connect(redMax_  , SIGNAL(valueChanged(double)), this, SLOT(modelRangeValueChanged(double)));
  connect(greenMin_, SIGNAL(valueChanged(double)), this, SLOT(modelRangeValueChanged(double)));
  connect(greenMax_, SIGNAL(valueChanged(double)), this, SLOT(modelRangeValueChanged(double)));
  connect(blueMin_ , SIGNAL(valueChanged(double)), this, SLOT(modelRangeValueChanged(double)));
  connect(blueMax_ , SIGNAL(valueChanged(double)), this, SLOT(modelRangeValueChanged(double)));

  //---

  modelGridLayout->setRowStretch(5, 1);

  stack_->addWidget(modelFrame);

  //---

  QFrame *definedFrame = new QFrame;

  definedFrame->setObjectName("definedFrame");

  QVBoxLayout *definedFrameLayout = new QVBoxLayout(definedFrame);
  definedFrameLayout->setMargin(2); definedFrameLayout->setSpacing(2);

  definedColors_ = new CQGradientPaletteDefinedColors;

  definedColors_->updateColors(pal);

  definedFrameLayout->addWidget(definedColors_);

  connect(definedColors_, SIGNAL(colorsChanged()), this, SLOT(colorsChanged()));

  QFrame *definedButtonsFrame = new QFrame;

  definedButtonsFrame->setObjectName("definedButtonsFrame");

  QHBoxLayout *definedButtonsLayout = new QHBoxLayout(definedButtonsFrame);
  definedButtonsLayout->setMargin(2); definedButtonsLayout->setSpacing(2);

  addColorButton_   = new QPushButton("Add");
  loadColorsButton_ = new QPushButton("Load");

  addColorButton_  ->setObjectName("add");
  loadColorsButton_->setObjectName("load");

  definedButtonsLayout->addWidget(addColorButton_);
  definedButtonsLayout->addWidget(loadColorsButton_);
  definedButtonsLayout->addStretch(1);

  connect(addColorButton_  , SIGNAL(clicked()), this, SLOT(addColorSlot()));
  connect(loadColorsButton_, SIGNAL(clicked()), this, SLOT(loadColorsSlot()));

  definedFrameLayout->addWidget(definedButtonsFrame);

  stack_->addWidget(definedFrame);

  //---

  QFrame *functionsFrame = new QFrame;

  functionsFrame->setObjectName("functionsFrame");

  QGridLayout *functionsGridLayout = new QGridLayout(functionsFrame);
  functionsGridLayout->setMargin(2); functionsGridLayout->setSpacing(2);

  createFunctionEdit(functionsGridLayout, 0, "Red"  , &redFunction_  );
  createFunctionEdit(functionsGridLayout, 1, "Green", &greenFunction_);
  createFunctionEdit(functionsGridLayout, 2, "Blue" , &blueFunction_ );

  functionsGridLayout->setRowStretch(3, 1);

  setRedFunction  (pal->redFunction  ());
  setGreenFunction(pal->greenFunction());
  setBlueFunction (pal->blueFunction ());

  connect(redFunction_  , SIGNAL(editingFinished()), this, SLOT(functionChanged()));
  connect(greenFunction_, SIGNAL(editingFinished()), this, SLOT(functionChanged()));
  connect(blueFunction_ , SIGNAL(editingFinished()), this, SLOT(functionChanged()));

  stack_->addWidget(functionsFrame);

  //---

  QFrame *cubeFrame = new QFrame;

  cubeFrame->setObjectName("cubeFrame");

  QGridLayout *cubeGridLayout = new QGridLayout(cubeFrame);
  cubeGridLayout->setMargin(2); cubeGridLayout->setSpacing(2);

  createRealEdit(cubeGridLayout, 0, "Start"     , &cubeStart_     );
  createRealEdit(cubeGridLayout, 1, "Cycles"    , &cubeCycles_    );
  createRealEdit(cubeGridLayout, 2, "Saturation", &cubeSaturation_);

  cubeNegativeCheck_ = new QCheckBox("Negative");

  cubeNegativeCheck_->setObjectName("negative");

  connect(cubeNegativeCheck_, SIGNAL(stateChanged(int)), this, SLOT(cubeNegativeChecked(int)));

  cubeGridLayout->addWidget(cubeNegativeCheck_, 3, 0, 1, 2);

  cubeGridLayout->setRowStretch(4, 1);

  setCubeStart     (pal->cbStart     ());
  setCubeCycles    (pal->cbCycles    ());
  setCubeSaturation(pal->cbSaturation());

  connect(cubeStart_     , SIGNAL(valueChanged(double)), this, SLOT(cubeValueChanged(double)));
  connect(cubeCycles_    , SIGNAL(valueChanged(double)), this, SLOT(cubeValueChanged(double)));
  connect(cubeSaturation_, SIGNAL(valueChanged(double)), this, SLOT(cubeValueChanged(double)));

  stack_->addWidget(cubeFrame);

  //---

  layout->addWidget(stack_);
  layout->addStretch(1);

  //---

  connect(this, SIGNAL(stateChanged()), palette, SLOT(update()));
}

void
CQGradientPaletteIFace::
colorTypeChanged(int)
{
  CGradientPalette *pal = palette_->gradientPalette();

  pal->setColorType(colorType_->type());

  if      (pal->colorType() == CGradientPalette::ColorType::MODEL)
    stack_->setCurrentIndex(0);
  else if (pal->colorType() == CGradientPalette::ColorType::DEFINED)
    stack_->setCurrentIndex(1);
  else if (pal->colorType() == CGradientPalette::ColorType::FUNCTIONS)
    stack_->setCurrentIndex(2);
  else if (pal->colorType() == CGradientPalette::ColorType::CUBEHELIX)
    stack_->setCurrentIndex(3);

  emit stateChanged();
}

void
CQGradientPaletteIFace::
colorModelChanged(int)
{
  CGradientPalette *pal = palette_->gradientPalette();

  pal->setColorModel(colorModel_->model());

  emit stateChanged();
}

int
CQGradientPaletteIFace::
redModel() const
{
  return redModel_->currentIndex();
}

void
CQGradientPaletteIFace::
setRedModel(int model)
{
  redModel_->setCurrentIndex(model);
}

int
CQGradientPaletteIFace::
greenModel() const
{
  return greenModel_->currentIndex();
}

void
CQGradientPaletteIFace::
setGreenModel(int model)
{
  greenModel_->setCurrentIndex(model);
}

int
CQGradientPaletteIFace::
blueModel() const
{
  return blueModel_->currentIndex();
}

void
CQGradientPaletteIFace::
setBlueModel(int model)
{
  blueModel_->setCurrentIndex(model);
}

void
CQGradientPaletteIFace::
modelChanged(int model)
{
  CQGradientPaletteModel *me = qobject_cast<CQGradientPaletteModel *>(sender());
  assert(me);

  CGradientPalette *pal = palette_->gradientPalette();

  if      (me == redModel_)
    pal->setRedModel(model);
  else if (me == greenModel_)
    pal->setGreenModel(model);
  else if (me == blueModel_)
    pal->setBlueModel(model);

  emit stateChanged();
}

void
CQGradientPaletteIFace::
modelRNegativeChecked(int state)
{
  CGradientPalette *pal = palette_->gradientPalette();

  pal->setRedNegative(state);

  emit stateChanged();
}

void
CQGradientPaletteIFace::
modelGNegativeChecked(int state)
{
  CGradientPalette *pal = palette_->gradientPalette();

  pal->setGreenNegative(state);

  emit stateChanged();
}

void
CQGradientPaletteIFace::
modelBNegativeChecked(int state)
{
  CGradientPalette *pal = palette_->gradientPalette();

  pal->setBlueNegative(state);

  emit stateChanged();
}

void
CQGradientPaletteIFace::
modelRangeValueChanged(double r)
{
  CQRealSpin *rs = qobject_cast<CQRealSpin *>(sender());
  assert(rs);

  CGradientPalette *pal = palette_->gradientPalette();

  if      (rs == redMin_)
    pal->setRedMin(r);
  else if (rs == redMax_)
    pal->setRedMax(r);

  emit stateChanged();
}

void
CQGradientPaletteIFace::
cubeNegativeChecked(int state)
{
  CGradientPalette *pal = palette_->gradientPalette();

  pal->setCubeNegative(state);

  emit stateChanged();
}

void
CQGradientPaletteIFace::
colorsChanged()
{
  CGradientPalette *pal = palette_->gradientPalette();

  pal->resetDefinedColors();

  for (int i = 0; i < definedColors_->numRealColors(); ++i) {
    const CQGradientPaletteDefinedColors::RealColor &realColor = definedColors_->realColor(i);

    pal->addDefinedColor(realColor.r, realColor.c);
  }

  emit stateChanged();
}

void
CQGradientPaletteIFace::
addColorSlot()
{
  CGradientPalette *pal = palette_->gradientPalette();

  int row = 0;

  QList<QTableWidgetItem *> selectedItems = definedColors_->selectedItems();

  if (selectedItems.length())
    row = selectedItems[0]->row();

  const CGradientPalette::ColorMap colors = pal->colors();

  double x = 0.5;
  CColor c = CRGBA(0.5, 0.5, 0.5);

  int row1 = -1;

  if      (row + 1 < int(colors.size()))
    row1 = row;
  else if (row < int(colors.size()) && row > 0)
    row1 = row - 1;

  if (row1 >= 0) {
    auto p = colors.begin();

    std::advance(p, row1);

    double        x1 = (*p).first;
    const CColor &c1 = (*p).second;

    ++p;
    double        x2 = (*p).first;
    const CColor &c2 = (*p).second;

    x = (x1 + x2)/2;
    c = CColor::interp(c1, c2, 0.5);
  }

  pal->addDefinedColor(x, c);

  definedColors_->updateColors(pal);

  emit stateChanged();
}

void
CQGradientPaletteIFace::
loadColorsSlot()
{
  CGradientPalette *pal = palette_->gradientPalette();

  QString dir = QDir::current().dirName();

  QString fileName = QFileDialog::getOpenFileName(this, "Open File", dir, "Files (*.*)");

  if (! fileName.length())
    return;

  pal->readFile(fileName.toStdString());

  definedColors_->updateColors(pal);
}

std::string
CQGradientPaletteIFace::
redFunction() const
{
  return redFunction_->text().toStdString();
}

void
CQGradientPaletteIFace::
setRedFunction(const std::string &fn)
{
  redFunction_->setText(fn.c_str());
}

std::string
CQGradientPaletteIFace::
greenFunction() const
{
  return greenFunction_->text().toStdString();
}

void
CQGradientPaletteIFace::
setGreenFunction(const std::string &fn)
{
  greenFunction_->setText(fn.c_str());
}

std::string
CQGradientPaletteIFace::
blueFunction() const
{
  return blueFunction_->text().toStdString();
}

void
CQGradientPaletteIFace::
setBlueFunction(const std::string &fn)
{
  blueFunction_->setText(fn.c_str());
}

void
CQGradientPaletteIFace::
functionChanged()
{
  QLineEdit *le = qobject_cast<QLineEdit *>(sender());
  assert(le);

  CGradientPalette *pal = palette_->gradientPalette();

  if      (le == redFunction_)
    pal->setRedFunction  (le->text().toStdString());
  else if (le == greenFunction_)
    pal->setGreenFunction(le->text().toStdString());
  else if (le == blueFunction_)
    pal->setBlueFunction (le->text().toStdString());

  emit stateChanged();
}

double
CQGradientPaletteIFace::
cubeStart() const
{
  return cubeStart_->value();
}

void
CQGradientPaletteIFace::
setCubeStart(double r)
{
  cubeStart_->setValue(r);
}

double
CQGradientPaletteIFace::
cubeCycles() const
{
  return cubeCycles_->value();
}

void
CQGradientPaletteIFace::
setCubeCycles(double r)
{
  cubeCycles_->setValue(r);
}

double
CQGradientPaletteIFace::
cubeSaturation() const
{
  return cubeSaturation_->value();
}

void
CQGradientPaletteIFace::
setCubeSaturation(double r)
{
  cubeSaturation_->setValue(r);
}

void
CQGradientPaletteIFace::
cubeValueChanged(double r)
{
  CQRealSpin *rs = qobject_cast<CQRealSpin *>(sender());
  assert(rs);

  CGradientPalette *pal = palette_->gradientPalette();

  if      (rs == cubeStart_)
    pal->setCbStart(r);
  else if (rs == cubeCycles_)
    pal->setCbCycles(r);
  else if (rs == cubeSaturation_)
    pal->setCbSaturation(r);

  emit stateChanged();
}

QFrame *
CQGradientPaletteIFace::
createColorTypeCombo(const QString &label, CQGradientPaletteColorType **type)
{
  QFrame *frame = new QFrame;

  frame->setObjectName("frame");

  QHBoxLayout *layout = new QHBoxLayout(frame);
  layout->setMargin(2); layout->setSpacing(2);

  *type = new CQGradientPaletteColorType;

  layout->addWidget(new QLabel(label));
  layout->addWidget(*type);
  layout->addStretch(1);

  return frame;
}

QFrame *
CQGradientPaletteIFace::
createColorModelCombo(const QString &label, CQGradientPaletteColorModel **model)
{
  QFrame *frame = new QFrame;

  frame->setObjectName("frame");

  QHBoxLayout *layout = new QHBoxLayout(frame);
  layout->setMargin(2); layout->setSpacing(2);

  *model = new CQGradientPaletteColorModel;

  layout->addWidget(new QLabel(label));
  layout->addWidget(*model);
  layout->addStretch(1);

  return frame;
}

void
CQGradientPaletteIFace::
createModelCombo(QGridLayout *grid, int row, const QString &label, CQGradientPaletteModel **model)
{
  *model = new CQGradientPaletteModel;

  grid->addWidget(new QLabel(label), row, 0);
  grid->addWidget(*model, row, 1);

  grid->setColumnStretch(2, true);
}

void
CQGradientPaletteIFace::
createFunctionEdit(QGridLayout *grid, int row, const QString &label, QLineEdit **edit)
{
  *edit = new QLineEdit;

  grid->addWidget(new QLabel(label), row, 0);
  grid->addWidget(*edit, row, 1);

  grid->setColumnStretch(2, true);
}

void
CQGradientPaletteIFace::
createRealEdit(QGridLayout *grid, int row, const QString &label, CQRealSpin **edit)
{
  *edit = new CQRealSpin;

  grid->addWidget(new QLabel(label), row, 0);
  grid->addWidget(*edit, row, 1);

  grid->setColumnStretch(2, true);
}

//---

CQGradientPaletteColorType::
CQGradientPaletteColorType(QWidget *parent) :
 QComboBox(parent)
{
  setObjectName("colorType");

  addItem("Model"     , QVariant(static_cast<int>(CGradientPalette::ColorType::MODEL    )));
  addItem("Defined"   , QVariant(static_cast<int>(CGradientPalette::ColorType::DEFINED  )));
  addItem("Functions" , QVariant(static_cast<int>(CGradientPalette::ColorType::FUNCTIONS)));
  addItem("Cube Helix", QVariant(static_cast<int>(CGradientPalette::ColorType::CUBEHELIX)));
}

CGradientPalette::ColorType
CQGradientPaletteColorType::
type() const
{
  return static_cast<CGradientPalette::ColorType>(itemData(currentIndex()).toInt());
}

void
CQGradientPaletteColorType::
setType(const CGradientPalette::ColorType &type)
{
  QVariant var(static_cast<int>(type));

  for (int i = 0; i < count(); ++i)
    if (itemData(i).toInt() == var.toInt())
      setCurrentIndex(i);
}

//---

CQGradientPaletteColorModel::
CQGradientPaletteColorModel(QWidget *parent) :
 QComboBox(parent)
{
  setObjectName("colorModel");

  addItem("RGB", QVariant(static_cast<int>(CGradientPalette::ColorModel::RGB)));
  addItem("HSV", QVariant(static_cast<int>(CGradientPalette::ColorModel::HSV)));
}

CGradientPalette::ColorModel
CQGradientPaletteColorModel::
model() const
{
  return static_cast<CGradientPalette::ColorModel>(itemData(currentIndex()).toInt());
}

void
CQGradientPaletteColorModel::
setModel(const CGradientPalette::ColorModel &model)
{
  QVariant var(static_cast<int>(model));

  for (int i = 0; i < count(); ++i)
    if (itemData(i).toInt() == var.toInt())
      setCurrentIndex(i);
}

//---

CQGradientPaletteModel::
CQGradientPaletteModel(QWidget *parent) :
 QComboBox(parent)
{
  setObjectName("model");

  for (int i = 0; i < CGradientPalette::numModels(); ++i)
    addItem(CGradientPalette::modelName(i).c_str());
}

//---

class CQGradientPaletteDefinedColorsDelegate : public QItemDelegate {
 public:
  CQGradientPaletteDefinedColorsDelegate(CQGradientPaletteDefinedColors *colors_);

  // Override to create editor
  QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                        const QModelIndex &index) const;

  // Override to get content from editor
  void setEditorData(QWidget *editor, const QModelIndex &index) const;

  // Override to set editor from content
  void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

  QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

  void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                            const QModelIndex &index) const;

  void paint(QPainter *painter, const QStyleOptionViewItem &option,
             const QModelIndex &index) const;

 private:
  CQGradientPaletteDefinedColors *colors_;
};

CQGradientPaletteDefinedColors::
CQGradientPaletteDefinedColors(QWidget *parent) :
 QTableWidget(parent)
{
  setObjectName("defineColors");

  verticalHeader()->hide();

  CQGradientPaletteDefinedColorsDelegate *delegate =
    new CQGradientPaletteDefinedColorsDelegate(this);

  setItemDelegate(delegate);
  setEditTriggers(QAbstractItemView::AllEditTriggers);

  setSelectionMode(QAbstractItemView::SingleSelection);
  setSelectionBehavior(QAbstractItemView::SelectRows);

  QHeaderView *header = horizontalHeader();

  header->setStretchLastSection(true) ;
}

void
CQGradientPaletteDefinedColors::
updateColors(CGradientPalette *palette)
{
  setColumnCount(2);

  setHorizontalHeaderLabels(QStringList() << "X" << "Color");

  setRowCount(palette->numColors());

  realColors_.clear();

  for (const auto &c : palette->colors())
    realColors_.emplace_back(c.first, c.second.rgba());

  for (int r = 0; r < numRealColors(); ++r) {
    const RealColor &realColor = this->realColor(r);

    QTableWidgetItem *item1 = new QTableWidgetItem(QString("%1").arg(realColor.r));
    QTableWidgetItem *item2 = new QTableWidgetItem(realColor.c.getRGB().toString().c_str());

    setItem(r, 0, item1);
    setItem(r, 1, item2);
  }
}

const CQGradientPaletteDefinedColors::RealColor &
CQGradientPaletteDefinedColors::
realColor(int r) const
{
  return realColors_[r];
}

void
CQGradientPaletteDefinedColors::
setRealColor(int r, const RealColor &realColor)
{
  realColors_[r] = realColor;

  //QTableWidgetItem *item1 = new QTableWidgetItem(QString("%1").arg(realColor.r));
  //QTableWidgetItem *item2 = new QTableWidgetItem(realColor.c.getRGB().toString().c_str());

  //setItem(r, 0, item1);
  //setItem(r, 1, item2);

  emit colorsChanged();
}

//---

CQGradientPaletteDefinedColorsDelegate::
CQGradientPaletteDefinedColorsDelegate(CQGradientPaletteDefinedColors *colors_) :
 QItemDelegate(colors_), colors_(colors_)
{
}

QWidget *
CQGradientPaletteDefinedColorsDelegate::
createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  QTableWidgetItem *item = colors_->item(index.row(), index.column());
  assert(item);

  const CQGradientPaletteDefinedColors::RealColor &realColor = colors_->realColor(index.row());

  if       (index.column() == 0) {
    CQRealSpin *edit = new CQRealSpin(parent);

    edit->setValue(realColor.r);

    return edit;
  }
  else if (index.column() == 1) {
    CQColorChooser *edit = new CQColorChooser(parent);

    edit->setStyles(CQColorChooser::Text | CQColorChooser::ColorButton);

    edit->setColor(QColor(realColor.c.getRedI(), realColor.c.getGreenI(), realColor.c.getBlueI()));

    return edit;
  }
  else
    return QItemDelegate::createEditor(parent, option, index);
}

// model->editor
void
CQGradientPaletteDefinedColorsDelegate::
setEditorData(QWidget *w, const QModelIndex &index) const
{
  QTableWidgetItem *item = colors_->item(index.row(), index.column());
  assert(item);

  const CQGradientPaletteDefinedColors::RealColor &realColor = colors_->realColor(index.row());

  if       (index.column() == 0) {
    CQRealSpin *edit = qobject_cast<CQRealSpin *>(w);

    edit->setValue(realColor.r);
  }
  else if (index.column() == 1) {
    CQColorChooser *edit = qobject_cast<CQColorChooser *>(w);

    const CRGBA &c = realColor.c;

    QColor qc(c.getRedI(), c.getGreenI(), c.getBlueI());

    edit->setColor(qc);
  }
  else
    QItemDelegate::setEditorData(w, index);
}

// editor->model
void
CQGradientPaletteDefinedColorsDelegate::
setModelData(QWidget *w, QAbstractItemModel *model, const QModelIndex &index) const
{
  QTableWidgetItem *item = colors_->item(index.row(), index.column());
  assert(item);

  CQGradientPaletteDefinedColors::RealColor realColor = colors_->realColor(index.row());

  if       (index.column() == 0) {
    CQRealSpin *edit = qobject_cast<CQRealSpin *>(w);

    double r = edit->value();

    model->setData(index, QVariant(r));

    realColor.r = r;
  }
  else if (index.column() == 1) {
    CQColorChooser *edit = qobject_cast<CQColorChooser *>(w);

    QColor qc = edit->color();

    realColor.c = CRGBA(qc.redF(), qc.greenF(), qc.blueF(), qc.alphaF());

    model->setData(index, QVariant(qc));
  }
  else
    QItemDelegate::setModelData(w, model, index);

  colors_->setRealColor(index.row(), realColor);
}

QSize
CQGradientPaletteDefinedColorsDelegate::
sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  QTableWidgetItem *item = colors_->item(index.row(), index.column());
  assert(item);

  return QItemDelegate::sizeHint(option, index);
}

void
CQGradientPaletteDefinedColorsDelegate::
updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                     const QModelIndex &/* index */) const
{
  editor->setGeometry(option.rect);
}

void
CQGradientPaletteDefinedColorsDelegate::
paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  QTableWidgetItem *item = colors_->item(index.row(), index.column());
  assert(item);

  QItemDelegate::paint(painter, option, index);
}
