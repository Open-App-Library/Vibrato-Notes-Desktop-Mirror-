#include "notelistmanager.h"
#include <QDebug>
#include <QAbstractItemView>
#include "../meta/db/notedatabase.h"
#include <helper-io.hpp>
#include "escribamanager.h"

NoteListManager::NoteListManager(CustomListView *view, QWidget *noteListAddons, Database *db, Manager *manager) :
  m_view(view),
  m_noteListAddons(noteListAddons),
  m_manager(manager),
  m_db(db)
{
  m_noteListAddonsUi = new Ui::NoteListAddonsWidget;
  m_noteListAddonsUi->setupUi( m_noteListAddons );

  // Hide description area for now. In the
  // future I would love to add notebook
  // description support.
  m_noteListAddonsUi->description->hide();

  m_filter = new NoteFilter( m_db );
  m_model = new NoteListModel(view, m_db->noteDatabase());
  m_proxyModel = new NoteListProxyModel(view, m_db);
  m_proxyModel->setSourceModel(m_model);
  m_proxyModel->sort(0, Qt::DescendingOrder);

  view->setModel(m_proxyModel);

  // Create special views
  m_trashView = new TrashView(m_db, m_manager, this);

  connect(m_manager, &Manager::ready,
          this, &NoteListManager::managerIsReady);

  connect(m_view, &CustomListView::selectedItemChanged,
          this, &NoteListManager::noteListItemChanged);

  connect(m_proxyModel, &NoteListProxyModel::invalidatedFilter,
          this, &NoteListManager::ensureCurrentNoteIsSelected);

  connect(m_db->notebookDatabase(), &NotebookDatabase::notebooksRemoved,
          this, &NoteListManager::notebooksDeleted);

  connect(m_db->tagDatabase(), &TagDatabase::tagRemoved,
          this, &NoteListManager::tagDeleted);

  connect(m_db->noteDatabase(), &NoteDatabase::noteFavoritedChanged,
          this, &NoteListManager::favoritedChanged);

  connect(m_db->noteDatabase(), &NoteDatabase::noteTrashedOrRestored,
          this, &NoteListManager::trashedOrRestored);

  connect(m_db->notebookDatabase(), &NotebookDatabase::notebookChanged,
          this, &NoteListManager::aNotebookChanged);
  connect(m_db->tagDatabase(), &TagDatabase::tagChanged,
          this, &NoteListManager::aTagChanged);

  // commented outff for now. This is potentially a more efficient way to set indexWidgets
  // however it has a slight graphical bug when loading a lot of notes.
  //    connect(m_proxyModel, &QSortFilterProxyModel::rowsInserted,
  //            this, &NoteListManager::rowsInsertedInProxy);
}

NoteListManager::~NoteListManager()
{
  delete m_filter;
  delete m_noteListAddonsUi;
}

QListView *NoteListManager::view() const
{
  return m_view;
}

NoteListModel *NoteListManager::model() const
{
  return m_model;
}

NoteListProxyModel *NoteListManager::proxyModel() const
{
  return m_proxyModel;
}

NoteListAddonsWidget *NoteListManager::addonsWidgetUi() const
{
  return m_noteListAddonsUi;
}

NoteListItem *NoteListManager::add_note(Note *note)
{
  NoteListItem *i = m_model->appendItem(note);
  return i;
}

void NoteListManager::remove_note(int index)
{
  m_model->removeRows(index, 1);
}

void NoteListManager::clear()
{
  m_model->clear();
}

void NoteListManager::filterOutEverything(bool shouldFilterOutEverything)
{
  hideAddons();
  m_proxyModel->filterOutEverything(shouldFilterOutEverything);
}

void NoteListManager::loadNotesFromNoteDatabase()
{
  loadNotesFromNoteDatabase( m_db->noteDatabase() );
}

void NoteListManager::loadNotesFromNoteDatabase(NoteDatabase *noteDatabase)
{
  clear();
  for (int i = 0; i < noteDatabase->size(); i++) {
    add_note(noteDatabase->list()[i]);
  }
}

void NoteListManager::openIndexInEditor(int index)
{
  if ( m_proxyModel->rowCount() == 0 || index >= m_proxyModel->rowCount() )
    return;
  m_view->setCurrentIndex( m_proxyModel->index(index,0) );
  m_manager->escribaManager()->setNote( m_proxyModel->item(index)->note() );
}

void NoteListManager::clearFilter(bool invalidate)
{
  m_proxyModel->clearFilter(invalidate);
}

void NoteListManager::addNotebookToFilter(Notebook *notebook)
{
  m_proxyModel->addNotebookToFilter(notebook);
}

void NoteListManager::addTagToFilter(Tag *tag)
{
  m_proxyModel->addTagToFilter(tag);
}

void NoteListManager::disconnectCurrentView() {
  ///
  // Notebook View Deactivation
  // If in a notebook view and the current notebook is not null, disconnect it.
  ///
  if ( m_curViewType == View_Notebook && m_curViewType_Notebook != nullptr ) {

  }
  ///
  // Tag View Deactivation
  // If in a tag view and the current tag is not null, disconnect it.
  ///
  else if ( m_curViewType == View_Tag && m_curViewType_Tag != nullptr ) {

  }
  ///
  // Trash View Deactivation
  ///
  else if ( m_curViewType == View_Trash ) {
    m_trashView->deactivateView();
  }
}

void NoteListManager::showAllNotesView()
{
  deselect();
  disconnectCurrentView();
  m_curViewType = View_AllNotes;
  hideAddons();
  clearFilter();
}

void NoteListManager::showFavoritesView()
{
  deselect();
  disconnectCurrentView();
  m_curViewType = View_Favorites;
  setTitle("Favorites");

  int favCount = 0;
  for ( Note *note : m_db->noteDatabase()->list() )
    if ( note->favorited() )
      favCount++;
  setMetrics(favCount, "favorite note");
  clearFilter(false);
  m_proxyModel->setFavoritesFilterMode(NoteListProxyModel::FavoritesOnly);
}

void NoteListManager::showTrashView()
{
  m_curViewType = View_Trash;
  m_trashView->activateView();
}

void NoteListManager::showNotebookView(Notebook *notebook)
{
  deselect();
  disconnectCurrentView();

  if ( notebook == nullptr )
    return;

  m_curViewType = View_Notebook;
  m_curViewType_ItemID = notebook->id();
  m_curViewType_Notebook = notebook;

  clearFilter(false);
  addNotebookToFilter(notebook);

  // Set the title label
  setTitle( notebook->title() );

  // Set the metricsLabel to the amount of notes the notebook
  // contains. It also checks with child notebook IDs.
  QVector<int> notebook_ids = {notebook->id()};
  QVector<Notebook*> children = notebook->recurseChildren();
  int noteCount = 0;
  for (Notebook *child : children)
    notebook_ids.append( child->id() );
  for (Note *note : m_db->noteDatabase()->list() )
    if ( notebook_ids.contains( note->notebook() ) )
      noteCount++;
  setMetrics(noteCount, "note");
}

void NoteListManager::showTagView(Tag *tag)
{
  deselect();
  disconnectCurrentView();

  if ( tag == nullptr )
    return;

  m_curViewType = View_Tag;
  m_curViewType_ItemID = tag->id();
  m_curViewType_Tag = tag;

  clearFilter(false);
  addTagToFilter(tag);

  setTitle( tag->title() );

  int tagCount = 0;
  for ( Note *note : m_db->noteDatabase()->list() )
    if ( note->tags().contains(tag->id()) )
      tagCount++;
  setMetrics(tagCount, "note");
}

void NoteListManager::showSearchView(QString searchQuery)
{
  deselect();
  disconnectCurrentView();
  m_curViewType = View_Search;
  hideMetrics();
  setTitle("Search results for STRING");
}

void NoteListManager::setTitle(QString title)
{
  m_noteListAddonsUi->title->setText(title);
  m_noteListAddons->show();
  m_noteListAddonsUi->title->show();
}

void NoteListManager::setMetrics(int count, QString objectTypeSingular, QString pluralOverride)
{
  QString objectString = objectTypeSingular;
  if ( count > 1 || count == 0 ) {
    if ( pluralOverride != QString("") )
      objectString = pluralOverride;
    else
      objectString.append('s');
  }
  QString label = QString("%1 %2").arg(HelperIO::numberToString(count), objectString);
  m_noteListAddons->show();
  m_noteListAddonsUi->metricsLabel->show();
  m_noteListAddonsUi->metricsLabel->setText(label);
}

void NoteListManager::hideTitle()
{
  m_noteListAddonsUi->title->hide();
}

void NoteListManager::hideMetrics()
{
  m_noteListAddonsUi->metricsLabel->hide();
}

void NoteListManager::hideAddons()
{
  m_noteListAddons->hide();
}

void NoteListManager::deselect()
{
  m_view->clearSelection();
  m_view->setCurrentIndex(QModelIndex());
}

void NoteListManager::noteListItemChanged(const QModelIndex &current_proxy, const QModelIndex &previous_proxy)
{
  QModelIndex current = m_proxyModel->mapToSource(current_proxy);
  QModelIndex previous = m_proxyModel->mapToSource(previous_proxy);
  if ( current == previous )
    return;
  if ( previous.isValid() && previous_proxy.isValid() ) {
    NoteListItem *prevItem = static_cast<NoteListItem*>(previous.internalPointer());
    prevItem->setSelectedStyle(false);
  }

  if ( current.isValid() && current_proxy.isValid() ) {
    NoteListItem *curItem = static_cast<NoteListItem*>(current.internalPointer());
    curItem->setSelectedStyle(true);
    emit selectedNote( curItem->note() );
  }
}

void NoteListManager::ensureCurrentNoteIsSelected()
{
  deselect();
  Note *note = m_manager->escribaManager()->note();
  for (int i=0; i<m_proxyModel->rowCount(); i++) {
    QModelIndex index = m_proxyModel->index(i, 0);
    QModelIndex sourceIndex = m_proxyModel->mapToSource(index);
    NoteListItem *curItem = static_cast<NoteListItem*>(sourceIndex.internalPointer());
    if ( curItem->note() == note ) {
      m_view->setCurrentIndex(index);
    }
  }
}

void NoteListManager::managerIsReady() {
  connect(m_manager->escribaManager(), &EscribaManager::deselected,
          this, &NoteListManager::escribaDeselected);
}

void NoteListManager::notebooksDeleted(QVector<int> notebookIDs) {
  // If the current notebook gets deleted, set the value to nullptr for safety.
  if ( m_curViewType == View_Notebook &&
      notebookIDs.contains(m_curViewType_ItemID) )
    m_curViewType_Notebook = nullptr;
}

void NoteListManager::tagDeleted(int tagID) {
  if ( m_curViewType == View_Tag &&
       m_curViewType_ItemID == tagID )
    m_curViewType_Tag = nullptr;
}

void NoteListManager::favoritedChanged(void) {
  int favFilterMode = m_proxyModel->favoritesFilter();
  if ( favFilterMode == NoteListProxyModel::FavoritesOnly ||
       favFilterMode == NoteListProxyModel::FavoritesExclude )
    m_proxyModel->invalidate();
}

void NoteListManager::trashedOrRestored(void) {
  int trashFilterMode = m_proxyModel->trashedFilter();
  if ( trashFilterMode == NoteListProxyModel::TrashHidden ||
       trashFilterMode == NoteListProxyModel::TrashOnly )
    m_proxyModel->invalidate();
}

void NoteListManager::escribaDeselected() {
  if ( m_proxyModel->rowCount() > 0 ) {
    openIndexInEditor(0);
  }
}

void NoteListManager::aNotebookChanged(Notebook* notebook) {
  if ( m_curViewType == View_Notebook && m_curViewType_Notebook == notebook )
    showNotebookView(notebook);
}

void NoteListManager::aTagChanged(Tag* tag) {
  if ( m_curViewType == View_Tag && m_curViewType_Tag == tag )
    showTagView(tag);
}

// This is potentially a more efficient way to set indexWidgets
// however it has a slight graphical bug when loading a lot of notes.
// void NoteListManager::rowsInsertedInProxy(const QModelIndex &parent, int start, int end)
// {
//   (void)parent;

//   for (int i = start; i <= end; i++) {
//     QModelIndex index = m_proxyModel->index(i,0);
//     QModelIndex sourceIndex = m_proxyModel->mapToSource(index);
//     if ( m_proxyModel->filterAcceptsRow(sourceIndex.row(), QModelIndex()) ) {
//       NoteListItem *item = static_cast<NoteListItem*>( sourceIndex.internalPointer() );
//       m_view->setIndexWidget(index, new NoteListItemWidget( item->note() ));
//     }
//   }
// }
