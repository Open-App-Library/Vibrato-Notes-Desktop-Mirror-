#include "trashview.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>
#include "../notelistmanager.h"
#include "../escribamanager.h"

TrashView::TrashView(Database *db, Manager *manager, QObject *parent) :
  GenericView(db, manager, parent)
{

}

void TrashView::setTrashNotes(QVector<Note*> trashNotes) {
  m_trashNotes = trashNotes;
}

void TrashView::addTrashNote(Note* trashNote) {
  m_trashNotes.append(trashNote);
}

void TrashView::activateView()
{
  ////////////////////
  // Get view ready //
  ////////////////////
  NoteListManager *nlm = manager()->noteListManager();

  nlm->deselect();
  nlm->disconnectCurrentView();
  nlm->setTitle("Trash");

  int trashCount = 0;
  for ( Note *note : db()->noteDatabase()->list() )
    if ( note->trashed() ) {
      addTrashNote(note);
      trashCount++;
    }
  nlm->setMetrics(trashCount, "note");

  nlm->clearFilter(false);
  proxyModel()->filterOutEverything();

  ///////////////////////
  // Populate the list //
  ///////////////////////
  // Hide the regular list view
  listView()->hide();

  // Get the note list widget's layout
  QVBoxLayout *notelistlayout = static_cast<QVBoxLayout*>(listView()->parentWidget()->layout());

  // Create a widget and layout for the mass actions
  QHBoxLayout *massActionsLayout = new QHBoxLayout();
  m_massActions = new QWidget();
  m_massActions->setLayout(massActionsLayout);
  m_massActions->setMinimumHeight(40);
  m_massActions->setMaximumHeight(40);

  // Create the buttons and add them to layout
  m_checkbox = new QCheckBox();
  m_massRestore = new QToolButton();
  m_massDelete = new QToolButton();

  m_checkbox->setToolTip(tr("Select or deselect all notes in trash."));
  m_massRestore->setToolTip(tr("Mass restore"));
  m_massRestore->setIcon( QIcon::fromTheme("document-revert") );
  m_massDelete->setToolTip(tr("Mass delete"));
  m_massDelete->setIcon( QIcon::fromTheme("window-close") );

  // Will determine if mass action buttons should be hidden (Nothing selected) or showing (at least one selected)
  determineMassActionVisibility();

  // Add buttons to layout
  massActionsLayout->addWidget(m_checkbox);
  massActionsLayout->addWidget(m_massRestore);
  massActionsLayout->addWidget(m_massDelete);

  // Create the trash list widget
  m_trashListWidget = new QListWidget();

  // Add the mass actions and trash list widget to screen
  notelistlayout->addWidget(m_massActions);
  notelistlayout->addWidget(m_trashListWidget);

  // Poplulate the trash list widget
  for ( Note *note : m_trashNotes) {
    addItem(note);
  }

  // Signals
  connect(m_checkbox, &QCheckBox::stateChanged,
          this, &TrashView::toggleMassCheckmark);
  connect(m_trashListWidget, &QListWidget::currentItemChanged,
          this, &TrashView::selectionChanged);

  // Show the trash list widget
  m_trashListWidget->show();
}

void TrashView::deactivateView()
{
  // Delete trash list widget
  delete m_massActions;
  delete m_trashListWidget;
  m_massActions = nullptr;
  m_trashListWidget = nullptr;

  // Clear lists
  m_trashItems.clear();
  m_trashNotes.clear();

  addonsWidgetUi()->buttonBox->hide();

  // Show the regular list view
  listView()->show();
}

TrashItem* TrashView::addItem(Note* note) {
  TrashItem *item = new TrashItem(note, m_trashListWidget);
  m_trashItems.append(item);
  connect(item, &TrashItem::itemCheckedOrUnchecked,
          this, &TrashView::itemCheckedOrUnchecked);
  return item;
}

void TrashView::select(TrashItem* item) {
  item->checkbox()->setChecked(true);
}

void TrashView::deselect(TrashItem* item) {
  item->checkbox()->setChecked(false);
}

void TrashView::selectAll() {
  for ( TrashItem *item : m_trashItems )
    select(item);
}

void TrashView::deselectAll() {
  for ( TrashItem *item : m_trashItems )
    deselect(item);
}

void TrashView::determineMassActionVisibility(void) {
  if ( m_selectedTrashItems.length() > 0 ) {
    m_massRestore->show();
    m_massDelete->show();
  }
  else {
    m_massRestore->hide();
    m_massDelete->hide();
  }
}

void TrashView::selectionChanged(QListWidgetItem* current, QListWidgetItem* previous) {
  TrashItem *trashItemCur = static_cast<TrashItem*>(current);
  manager()->escribaManager()->setNote( trashItemCur->note() );
}

void TrashView::toggleMassCheckmark(void) {
  bool checked = m_checkbox->isChecked();
  if ( checked )
    selectAll();
  else
    deselectAll();
}

void TrashView::itemCheckedOrUnchecked(TrashItem *item) {
  if ( item->checked() ) {
    if ( !m_selectedTrashItems.contains(item) )
      m_selectedTrashItems.append(item);
  }
  else {
    if ( m_selectedTrashItems.contains(item) )
      m_selectedTrashItems.removeAll(item);
  }
  determineMassActionVisibility();
}