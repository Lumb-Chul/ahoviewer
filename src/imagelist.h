#ifndef _IMAGELIST_H_
#define _IMAGELIST_H_

#include <gtkmm.h>
#include <memory>
#include <queue>
#include <string>
#include <vector>

#include "archive/archive.h"
#include "booru/xml.h"
#include "image.h"

namespace AhoViewer
{
    class ImageList : public sigc::trackable
    {
        using ImageVector = std::vector<std::shared_ptr<Image>>;

        // This signal is emitted when m_Index is changed.
        // It is connected by the MainWindow which tells the ImageBox to draw the new image.
        using SignalChangedType = sigc::signal<void, const std::shared_ptr<Image>&>;

        // Emitted when AutoOpenArchive is true and loading an archive fails.
        using SignalArchiveErrorType = sigc::signal<void, const std::string>;

        // Used for async thumbnail pixbuf loading
        using PixbufPair = std::pair<size_t, const Glib::RefPtr<Gdk::Pixbuf>>;
    public:
        // ImageList::Widget {{{
        // This is used by ThumbnailBar and Booru::Page.
        class Widget
        {
            friend class ImageList;

            // When the widget's selected item changes it will emit this signal.
            using SignalSelectedChangedType = sigc::signal<void, const size_t>;
        public:
            virtual ~Widget() = default;
        protected:
            struct ModelColumns : public Gtk::TreeModelColumnRecord
            {
                ModelColumns() { add(pixbuf_column); }
                Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>> pixbuf_column;
            };

            virtual void set_selected(const size_t) = 0;
            virtual void scroll_to_selected() = 0;

            virtual void clear()
            {
                m_ListStore->clear();
            }
            virtual void set_pixbuf(const size_t index, const Glib::RefPtr<Gdk::Pixbuf> &pixbuf)
            {
                Gtk::TreeIter it = m_ListStore->get_iter(std::to_string(index));
                if (it) it->set_value(0, pixbuf);
            }
            void reserve(const size_t s)
            {
                for (size_t i = 0; i < s; ++i)
                    m_ListStore->append();
            }

            Glib::RefPtr<Gtk::ListStore> m_ListStore;
            SignalSelectedChangedType m_SignalSelectedChanged;
        private:
            void erase(const size_t i)
            {
                Gtk::TreeIter it = m_ListStore->get_iter(std::to_string(i));
                if (it) m_ListStore->erase(it);
            }
            void insert(const size_t i, const Glib::RefPtr<Gdk::Pixbuf> &pixbuf)
            {
                Gtk::TreeIter it = m_ListStore->get_iter(std::to_string(i));
                it = it ? m_ListStore->insert(it) : m_ListStore->append();
                it->set_value(0, pixbuf);
            }

            SignalSelectedChangedType signal_selected_changed() const { return m_SignalSelectedChanged; }
        };
        // }}}

        ImageList(Widget *const w);
        virtual ~ImageList();

        virtual void clear();
        bool load(const Glib::ustring path, std::string &error, int index = 0);

        // Action callbacks {{{
        void go_next();
        void go_previous();
        void go_first();
        void go_last();
        // }}}

        bool can_go_next() const;
        bool can_go_previous() const;

        virtual size_t get_size() const { return m_Images.size(); }

        size_t get_index() const { return m_Index; }
        const std::shared_ptr<Image>& get_current() const { return m_Images[m_Index]; }
        const Archive& get_archive() const { return *m_Archive; }
        bool empty() const { return m_Images.empty(); }
        bool from_archive() const { return !!m_Archive; }

        ImageVector::iterator begin() { return m_Images.begin(); }
        ImageVector::iterator end() { return m_Images.end(); }

        void on_cache_size_changed();

        SignalChangedType signal_changed() const { return m_SignalChanged; }
        SignalArchiveErrorType signal_archive_error() const { return m_SignalArchiveError; }
        sigc::signal<void> signal_cleared() const { return m_SignalCleared; }
        sigc::signal<void> signal_load_success() const { return m_SignalLoadSuccess; }
        sigc::signal<void> signal_size_changed() const { return m_SignalSizeChanged; }
    protected:
        virtual void set_current(const size_t index, const bool fromWidget = false, const bool force = false);
        virtual void load_thumbnails();

        Widget *const m_Widget;
        ImageVector m_Images;
        size_t m_Index;

        Glib::RefPtr<Gio::Cancellable> m_ThumbnailCancel;
        Glib::Threads::Thread *m_ThumbnailThread;

        SignalChangedType m_SignalChanged;
    private:
        void reset();
        template <typename T>
        std::vector<std::string> get_entries(const std::string &path);

        void on_thumbnail_loaded();
        void on_thumbnails_loaded();
        void on_directory_changed(const Glib::RefPtr<Gio::File> &file,
                                  const Glib::RefPtr<Gio::File>&,
                                  Gio::FileMonitorEvent event);

        void set_current_relative(const int d);
        void update_cache();
        void cancel_cache();

        std::vector<size_t> m_Cache;
        std::unique_ptr<Archive> m_Archive;
        std::vector<std::string> m_ArchiveEntries;
        std::queue<PixbufPair> m_ThumbnailQueue;
        std::function<int(size_t, size_t)> m_IndexSort;

        Glib::RefPtr<Gio::Cancellable> m_CacheCancel;
        Glib::Threads::Mutex m_ThumbnailMutex;
        Glib::Threads::Thread *m_CacheThread;
        Glib::RefPtr<Gio::FileMonitor> m_FileMonitor;

        Glib::Dispatcher m_SignalThumbnailLoaded,
                         m_SignalThumbnailsLoaded;

        sigc::connection m_ThumbnailLoadedConn;

        SignalArchiveErrorType      m_SignalArchiveError;
        sigc::signal<void>          m_SignalCleared,
                                    m_SignalLoadSuccess,
                                    m_SignalSizeChanged;
    };
}

#endif /* _IMAGELIST_H_ */
