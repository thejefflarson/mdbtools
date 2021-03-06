/* GTK - The GIMP Toolkit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

/*
 * Modified by the GTK+ Team and others 1997-1999.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/. 
 */

#include "gtkhlist.h"
#include "gtk/gtklistitem.h"
#include "gtk/gtkmain.h"
#include "gtk/gtksignal.h"
#include "gtk/gtklabel.h"

enum {
  SELECTION_CHANGED,
  SELECT_CHILD,
  UNSELECT_CHILD,
  LAST_SIGNAL
};

enum {
  ARG_0,
  ARG_SELECTION_MODE
};

#define SCROLL_TIME  100

/** GtkHList Methods **/
static void gtk_hlist_class_init	     (GtkHListClass   *klass);
static void gtk_hlist_init	     (GtkHList	     *list);
static void gtk_hlist_set_arg         (GtkObject      *object,
				      GtkArg         *arg,
				      guint           arg_id);
static void gtk_hlist_get_arg         (GtkObject      *object,
				      GtkArg         *arg,
				      guint           arg_id);
/** GtkObject Methods **/
static void gtk_hlist_shutdown	     (GtkObject	     *object);

/** GtkWidget Methods **/
static void gtk_hlist_size_request    (GtkWidget	     *widget,
				      GtkRequisition *requisition);
static void gtk_hlist_size_allocate   (GtkWidget	     *widget,
				      GtkAllocation  *allocation);
static void gtk_hlist_realize	     (GtkWidget	     *widget);
static void gtk_hlist_map	     (GtkWidget	     *widget);
static void gtk_hlist_unmap	     (GtkWidget	     *widget);
static void gtk_hlist_style_set	     (GtkWidget      *widget,
				      GtkStyle       *previous_style);
static void gtk_hlist_draw	     (GtkWidget	     *widget,
				      GdkRectangle   *area);
static gint gtk_hlist_expose	     (GtkWidget	     *widget,
				      GdkEventExpose *event);
static gint gtk_hlist_motion_notify   (GtkWidget      *widget,
				      GdkEventMotion *event);
static gint gtk_hlist_button_press    (GtkWidget      *widget,
				      GdkEventButton *event);
static gint gtk_hlist_button_release  (GtkWidget	     *widget,
				      GdkEventButton *event);

/** GtkContainer Methods **/
static void gtk_hlist_add	     (GtkContainer     *container,
				      GtkWidget        *widget);
static void gtk_hlist_remove	     (GtkContainer     *container,
				      GtkWidget        *widget);
static void gtk_hlist_forall	     (GtkContainer     *container,
				      gboolean          include_internals,
				      GtkCallback       callback,
				      gpointer          callback_data);
static GtkType gtk_hlist_child_type   (GtkContainer     *container);
static void gtk_hlist_set_focus_child (GtkContainer     *container,
				      GtkWidget        *widget);
static gint gtk_hlist_focus           (GtkContainer     *container,
				      GtkDirectionType  direction);

/** GtkHList Private Functions **/
static void gtk_hlist_move_focus_child      (GtkHList       *list,
					    GtkScrollType  scroll_type,
					    gfloat         position);
static gint gtk_hlist_horizontal_timeout    (GtkWidget     *list);
static gint gtk_hlist_vertical_timeout      (GtkWidget     *list);
static void gtk_hlist_remove_items_internal (GtkHList       *list,
					    GList         *items,
					    gboolean       no_unref);

/** GtkHList Selection Methods **/
static void gtk_real_list_select_child	        (GtkHList   *list,
						 GtkWidget *child);
static void gtk_real_list_unselect_child        (GtkHList   *list,
						 GtkWidget *child);

/** GtkHList Selection Functions **/
static void gtk_hlist_set_anchor                 (GtkHList   *list,
					         gboolean   add_mode,
					         gint       anchor,
					         GtkWidget *undo_focus_child);
static void gtk_hlist_fake_unselect_all          (GtkHList   *list,
			                         GtkWidget *item);
static void gtk_hlist_fake_toggle_row            (GtkHList   *list,
					         GtkWidget *item);
static void gtk_hlist_update_extended_selection  (GtkHList   *list,
					         gint       row);
static void gtk_hlist_reset_extended_selection   (GtkHList   *list);

/** GtkListItem Signal Functions **/
static void gtk_hlist_signal_drag_begin         (GtkWidget      *widget,
						GdkDragContext *context,
						GtkHList        *list);
static void gtk_hlist_signal_toggle_focus_row   (GtkListItem   *list_item,
						GtkHList       *list);
static void gtk_hlist_signal_select_all         (GtkListItem   *list_item,
						GtkHList       *list);
static void gtk_hlist_signal_unselect_all       (GtkListItem   *list_item,
						GtkHList       *list);
static void gtk_hlist_signal_undo_selection     (GtkListItem   *list_item,
						GtkHList       *list);
static void gtk_hlist_signal_start_selection    (GtkListItem   *list_item,
						GtkHList       *list);
static void gtk_hlist_signal_end_selection      (GtkListItem   *list_item,
						GtkHList       *list);
static void gtk_hlist_signal_extend_selection   (GtkListItem   *list_item,
						GtkScrollType  scroll_type,
						gfloat         position,
						gboolean       auto_start_selection,
						GtkHList       *list);
static void gtk_hlist_signal_scroll_horizontal  (GtkListItem   *list_item,
						GtkScrollType  scroll_type,
						gfloat         position,
						GtkHList       *list);
static void gtk_hlist_signal_scroll_vertical    (GtkListItem   *list_item,
						GtkScrollType  scroll_type,
						gfloat         position,
						GtkHList       *list);
static void gtk_hlist_signal_toggle_add_mode    (GtkListItem   *list_item,
						GtkHList       *list);
static void gtk_hlist_signal_item_select        (GtkListItem   *list_item,
						GtkHList       *list);
static void gtk_hlist_signal_item_deselect      (GtkListItem   *list_item,
						GtkHList       *list);
static void gtk_hlist_signal_item_toggle        (GtkListItem   *list_item,
						GtkHList       *list);


static void gtk_hlist_drag_begin (GtkWidget      *widget,
				 GdkDragContext *context);


static GtkContainerClass *parent_class = NULL;
static guint list_signals[LAST_SIGNAL] = { 0 };

static const gchar *vadjustment_key = "gtk-vadjustment";
static guint        vadjustment_key_id = 0;
static const gchar *hadjustment_key = "gtk-hadjustment";
static guint        hadjustment_key_id = 0;

GtkType
gtk_hlist_get_type (void)
{
  static GtkType list_type = 0;

  if (!list_type)
    {
      static const GtkTypeInfo list_info =
      {
	"GtkHList",
	sizeof (GtkHList),
	sizeof (GtkHListClass),
	(GtkClassInitFunc) gtk_hlist_class_init,
	(GtkObjectInitFunc) gtk_hlist_init,
	/* reserved_1 */ NULL,
	/* reserved_2 */ NULL,
        (GtkClassInitFunc) NULL,
      };

      list_type = gtk_type_unique (GTK_TYPE_CONTAINER, &list_info);
    }

  return list_type;
}

static void
gtk_hlist_class_init (GtkHListClass *class)
{
  GtkObjectClass *object_class;
  GtkWidgetClass *widget_class;
  GtkContainerClass *container_class;

  object_class = (GtkObjectClass*) class;
  widget_class = (GtkWidgetClass*) class;
  container_class = (GtkContainerClass*) class;

  parent_class = gtk_type_class (GTK_TYPE_CONTAINER);

  vadjustment_key_id = g_quark_from_static_string (vadjustment_key);
  hadjustment_key_id = g_quark_from_static_string (hadjustment_key);

  list_signals[SELECTION_CHANGED] =
    gtk_signal_new ("selection_changed",
		    GTK_RUN_FIRST,
		    object_class->type,
		    GTK_SIGNAL_OFFSET (GtkHListClass, selection_changed),
		    gtk_marshal_NONE__NONE,
		    GTK_TYPE_NONE, 0);
  list_signals[SELECT_CHILD] =
    gtk_signal_new ("select_child",
		    GTK_RUN_FIRST,
		    object_class->type,
		    GTK_SIGNAL_OFFSET (GtkHListClass, select_child),
		    gtk_marshal_NONE__POINTER,
		    GTK_TYPE_NONE, 1,
		    GTK_TYPE_WIDGET);
  list_signals[UNSELECT_CHILD] =
    gtk_signal_new ("unselect_child",
		    GTK_RUN_FIRST,
		    object_class->type,
		    GTK_SIGNAL_OFFSET (GtkHListClass, unselect_child),
		    gtk_marshal_NONE__POINTER,
		    GTK_TYPE_NONE, 1,
		    GTK_TYPE_WIDGET);

  gtk_object_class_add_signals (object_class, list_signals, LAST_SIGNAL);

  object_class->shutdown = gtk_hlist_shutdown;
  object_class->set_arg = gtk_hlist_set_arg;
  object_class->get_arg = gtk_hlist_get_arg;

  widget_class->map = gtk_hlist_map;
  widget_class->unmap = gtk_hlist_unmap;
  widget_class->style_set = gtk_hlist_style_set;
  widget_class->realize = gtk_hlist_realize;
  widget_class->draw = gtk_hlist_draw;
  widget_class->expose_event = gtk_hlist_expose;
  widget_class->button_press_event = gtk_hlist_button_press;
  widget_class->button_release_event = gtk_hlist_button_release;
  widget_class->motion_notify_event = gtk_hlist_motion_notify;
  widget_class->size_request = gtk_hlist_size_request;
  widget_class->size_allocate = gtk_hlist_size_allocate;
  widget_class->drag_begin = gtk_hlist_drag_begin;

  container_class->add = gtk_hlist_add;
  container_class->remove = gtk_hlist_remove;
  container_class->forall = gtk_hlist_forall;
  container_class->child_type = gtk_hlist_child_type;
  container_class->set_focus_child = gtk_hlist_set_focus_child;
  container_class->focus = gtk_hlist_focus;

  class->selection_changed = NULL;
  class->select_child = gtk_real_list_select_child;
  class->unselect_child = gtk_real_list_unselect_child;
  
  gtk_object_add_arg_type ("GtkHList::selection_mode",
			   GTK_TYPE_SELECTION_MODE, GTK_ARG_READWRITE,
			   ARG_SELECTION_MODE);
}

static void
gtk_hlist_init (GtkHList *list)
{
  list->children = NULL;
  list->selection = NULL;

  list->undo_selection = NULL;
  list->undo_unselection = NULL;

  list->last_focus_child = NULL;
  list->undo_focus_child = NULL;

  list->htimer = 0;
  list->vtimer = 0;

  list->anchor = -1;
  list->drag_pos = -1;
  list->anchor_state = GTK_STATE_SELECTED;

  list->selection_mode = GTK_SELECTION_SINGLE;
  list->drag_selection = FALSE;
  list->add_mode = FALSE;
}

static void
gtk_hlist_set_arg (GtkObject      *object,
		  GtkArg         *arg,
		  guint           arg_id)
{
  GtkHList *list = GTK_HLIST (object);
  
  switch (arg_id)
    {
    case ARG_SELECTION_MODE:
      gtk_hlist_set_selection_mode (list, GTK_VALUE_ENUM (*arg));
      break;
    }
}

static void
gtk_hlist_get_arg (GtkObject      *object,
		  GtkArg         *arg,
		  guint           arg_id)
{
  GtkHList *list = GTK_HLIST (object);
  
  switch (arg_id)
    {
    case ARG_SELECTION_MODE: 
      GTK_VALUE_ENUM (*arg) = list->selection_mode; 
      break;
    default:
      arg->type = GTK_TYPE_INVALID;
      break;
    }
}

GtkWidget*
gtk_hlist_new (void)
{
  return GTK_WIDGET (gtk_type_new (GTK_TYPE_HLIST));
}


/* Private GtkObject Methods :
 * 
 * gtk_hlist_shutdown
 */
static void
gtk_hlist_shutdown (GtkObject *object)
{
  gtk_hlist_clear_items (GTK_HLIST (object), 0, -1);
  GTK_OBJECT_CLASS (parent_class)->shutdown (object);
}


/* Private GtkWidget Methods :
 * 
 * gtk_hlist_size_request
 * gtk_hlist_size_allocate
 * gtk_hlist_realize
 * gtk_hlist_map
 * gtk_hlist_unmap
 * gtk_hlist_motion_notify
 * gtk_hlist_button_press
 * gtk_hlist_button_release
 */
static void
gtk_hlist_size_request (GtkWidget      *widget,
		       GtkRequisition *requisition)
{
  GtkHList *list;
  GtkWidget *child;
  GList *children;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_HLIST (widget));
  g_return_if_fail (requisition != NULL);

  list = GTK_HLIST (widget);
  requisition->width = 0;
  requisition->height = 0;

  children = list->children;
  while (children)
    {
      child = children->data;
      children = children->next;

      if (GTK_WIDGET_VISIBLE (child))
	{
	  GtkRequisition child_requisition;
	  
	  gtk_widget_size_request (child, &child_requisition);

	  requisition->width = MAX (requisition->width,
				    child_requisition.width);
	  requisition->height += child_requisition.height;
	}
    }

  requisition->width += GTK_CONTAINER (list)->border_width * 2;
  requisition->height += GTK_CONTAINER (list)->border_width * 2;

  requisition->width = MAX (requisition->width, 1);
  requisition->height = MAX (requisition->height, 1);
}

static void
gtk_hlist_size_allocate (GtkWidget     *widget,
			GtkAllocation *allocation)
{
  GtkHList *list;
  GtkWidget *child;
  GtkAllocation child_allocation;
  GList *children;
  gint max_child_height = 0;
  gint max_child_width = 0;
  gint rows = 0;
  gint column = 0;
  gint child_num = 0;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_HLIST (widget));
  g_return_if_fail (allocation != NULL);

  list = GTK_HLIST (widget);

  widget->allocation = *allocation;
  if (GTK_WIDGET_REALIZED (widget))
    gdk_window_move_resize (widget->window,
			    allocation->x, allocation->y,
			    allocation->width, allocation->height);

  if (list->children)
    {
      child_allocation.x = GTK_CONTAINER (list)->border_width;
      child_allocation.y = GTK_CONTAINER (list)->border_width;
      child_allocation.width = MAX (1, (gint)allocation->width -
				    child_allocation.x * 2);

      /* find maximum height and width from all children */
      children = list->children;

      while (children)
	{
	  child = children->data;
	  children = children->next;

	  if (GTK_WIDGET_VISIBLE (child))
	    {
	      GtkRequisition child_requisition;
	      gtk_widget_get_child_requisition (child, &child_requisition);
	      
              if (child_requisition.height > max_child_height)
                 max_child_height = child_requisition.height;
              if (child_requisition.width > max_child_width)
                 max_child_width = child_requisition.width;
	    }
	}

      rows = allocation->height / max_child_height;
              
      children = list->children;

      while (children)
	{
	  child = children->data;
	  children = children->next;

	  if (GTK_WIDGET_VISIBLE (child))
	    {
	      child_allocation.height = max_child_height;
	      child_allocation.width = max_child_width;

	      gtk_widget_size_allocate (child, &child_allocation);

              child_allocation.y += max_child_height;
              
              child_num ++;
              column = (child_num) / rows;
	      child_allocation.x = column * max_child_width;

              if (child_num % rows == 0) {
                 child_allocation.y = GTK_CONTAINER (list)->border_width;
              }

	    }
	}
    }
}

static void
gtk_hlist_realize (GtkWidget *widget)
{
  GdkWindowAttr attributes;
  gint attributes_mask;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_HLIST (widget));

  GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);

  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.x = widget->allocation.x;
  attributes.y = widget->allocation.y;
  attributes.width = widget->allocation.width;
  attributes.height = widget->allocation.height;
  attributes.wclass = GDK_INPUT_OUTPUT;
  attributes.visual = gtk_widget_get_visual (widget);
  attributes.colormap = gtk_widget_get_colormap (widget);
  attributes.event_mask = gtk_widget_get_events (widget) | GDK_EXPOSURE_MASK;

  attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

  widget->window = gdk_window_new (gtk_widget_get_parent_window (widget),
				   &attributes, attributes_mask);
  gdk_window_set_user_data (widget->window, widget);

  widget->style = gtk_style_attach (widget->style, widget->window);
  gdk_window_set_background (widget->window, 
			     &widget->style->base[GTK_STATE_NORMAL]);
}

static void
gtk_hlist_map (GtkWidget *widget)
{
  GtkHList *list;
  GtkWidget *child;
  GList *children;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_HLIST (widget));

  GTK_WIDGET_SET_FLAGS (widget, GTK_MAPPED);
  list = GTK_HLIST (widget);

  children = list->children;
  while (children)
    {
      child = children->data;
      children = children->next;

      if (GTK_WIDGET_VISIBLE (child) &&
	  !GTK_WIDGET_MAPPED (child))
	gtk_widget_map (child);
    }

  gdk_window_show (widget->window);
}

static void
gtk_hlist_unmap (GtkWidget *widget)
{
  GtkHList *list;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_HLIST (widget));

  if (!GTK_WIDGET_MAPPED (widget))
    return;

  list = GTK_HLIST (widget);

  GTK_WIDGET_UNSET_FLAGS (widget, GTK_MAPPED);

  if (gdk_pointer_is_grabbed () && GTK_WIDGET_HAS_GRAB (list))
    {
      gtk_hlist_end_drag_selection (list);

      if (list->anchor != -1 && list->selection_mode == GTK_SELECTION_EXTENDED)
	gtk_hlist_end_selection (list);
    }

  gdk_window_hide (widget->window);
}

static gint
gtk_hlist_motion_notify (GtkWidget      *widget,
			GdkEventMotion *event)
{
  GtkHList *list;
  GtkWidget *item = NULL;
  GtkAdjustment *adj;
  GtkContainer *container;
  GList *work;
  gint x;
  gint y;
  gint row = -1;
  gint focus_row = 0;
  gint length = 0;

  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_HLIST (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  list = GTK_HLIST (widget);

  if (!list->drag_selection || !list->children)
    return FALSE;

  container = GTK_CONTAINER (widget);

  if (event->is_hint || event->window != widget->window)
    gdk_window_get_pointer (widget->window, &x, &y, NULL);

  adj = gtk_object_get_data_by_id (GTK_OBJECT (list), hadjustment_key_id);

  /* horizontal autoscrolling */
  if (adj && widget->allocation.width > adj->page_size &&
      (x < adj->value || x >= adj->value + adj->page_size))
    {
      if (list->htimer == 0)
	{
	  list->htimer = gtk_timeout_add
	    (SCROLL_TIME, (GtkFunction) gtk_hlist_horizontal_timeout, widget);
	  
	  if (!((x < adj->value && adj->value <= 0) ||
		(x > adj->value + adj->page_size &&
		 adj->value >= adj->upper - adj->page_size)))
	    {
	      gfloat value;

	      if (x < adj->value)
		value = adj->value + (x - adj->value) / 2 - 1;
	      else
		value = adj->value + 1 + (x - adj->value - adj->page_size) / 2;

	      gtk_adjustment_set_value (adj,
					CLAMP (value, 0.0,
					       adj->upper - adj->page_size));
	    }
	}
      else
	return FALSE;
    }

  
  /* vertical autoscrolling */
  for (work = list->children; work; length++, work = work->next)
    {
      if (row < 0)
	{
	  item = GTK_WIDGET (work->data);
	  if (item->allocation.y > y || 
	      (item->allocation.y <= y &&
	       item->allocation.y + item->allocation.height > y))
	    row = length;
	}

      if (work->data == container->focus_child)
	focus_row = length;
    }
  
  if (row < 0)
    row = length - 1;

  if (list->vtimer != 0)
    return FALSE;

  if (!((y < 0 && focus_row == 0) ||
	(y > widget->allocation.height && focus_row >= length - 1)))
    list->vtimer = gtk_timeout_add (SCROLL_TIME,
				    (GtkFunction) gtk_hlist_vertical_timeout,
				    list);

  if (row != focus_row)
    gtk_widget_grab_focus (item);

  switch (list->selection_mode)
    {
    case GTK_SELECTION_BROWSE:
      gtk_hlist_select_child (list, item);
      break;
    case GTK_SELECTION_EXTENDED:
      gtk_hlist_update_extended_selection (list, row);
      break;
    default:
      break;
    }

  return FALSE;
}

static gint
gtk_hlist_button_press (GtkWidget      *widget,
		       GdkEventButton *event)
{
  GtkHList *list;
  GtkWidget *item;

  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_HLIST (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  if (event->button != 1)
    return FALSE;

  list = GTK_HLIST (widget);
  item = gtk_get_event_widget ((GdkEvent*) event);

  while (item && !GTK_IS_LIST_ITEM (item))
    item = item->parent;

  if (item && (item->parent == widget))
    {
      gint last_focus_row;
      gint focus_row;

      if (event->type == GDK_BUTTON_PRESS)
	{
	  if (gdk_pointer_grab (widget->window, TRUE,
				GDK_POINTER_MOTION_HINT_MASK |
				GDK_BUTTON1_MOTION_MASK |
				GDK_BUTTON_RELEASE_MASK,
				NULL, NULL, event->time))
	    return FALSE;
	  
	  gtk_grab_add (widget);
	  list->drag_selection = TRUE;
	}
      else if (gdk_pointer_is_grabbed () && GTK_WIDGET_HAS_GRAB (list))
	gtk_hlist_end_drag_selection (list);
	  
      if (!GTK_WIDGET_HAS_FOCUS(item))
	gtk_widget_grab_focus (item);

      if (list->add_mode)
	{
	  list->add_mode = FALSE;
	  gtk_widget_queue_draw (item);
	}
      
      switch (list->selection_mode)
	{
	case GTK_SELECTION_SINGLE:
	case GTK_SELECTION_MULTIPLE:
	  if (event->type != GDK_BUTTON_PRESS)
	    gtk_hlist_select_child (list, item);
	  else
	    list->undo_focus_child = item;
	  break;
	  
	case GTK_SELECTION_BROWSE:
	  break;

	case GTK_SELECTION_EXTENDED:
	  focus_row = g_list_index (list->children, item);

	  if (list->last_focus_child)
	    last_focus_row = g_list_index (list->children,
					   list->last_focus_child);
	  else
	    {
	      last_focus_row = focus_row;
	      list->last_focus_child = item;
	    }

	  if (event->type != GDK_BUTTON_PRESS)
	    {
	      if (list->anchor >= 0)
		{
		  gtk_hlist_update_extended_selection (list, focus_row);
		  gtk_hlist_end_selection (list);
		}
	      gtk_hlist_select_child (list, item);
	      break;
	    }
	      
	  if (event->state & GDK_CONTROL_MASK)
	    {
	      if (event->state & GDK_SHIFT_MASK)
		{
		  if (list->anchor < 0)
		    {
		      g_list_free (list->undo_selection);
		      g_list_free (list->undo_unselection);
		      list->undo_selection = NULL;
		      list->undo_unselection = NULL;

		      list->anchor = last_focus_row;
		      list->drag_pos = last_focus_row;
		      list->undo_focus_child = list->last_focus_child;
		    }
		  gtk_hlist_update_extended_selection (list, focus_row);
		}
	      else
		{
		  if (list->anchor < 0)
		    gtk_hlist_set_anchor (list, TRUE,
					 focus_row, list->last_focus_child);
		  else
		    gtk_hlist_update_extended_selection (list, focus_row);
		}
	      break;
	    }

	  if (event->state & GDK_SHIFT_MASK)
	    {
	      gtk_hlist_set_anchor (list, FALSE,
				   last_focus_row, list->last_focus_child);
	      gtk_hlist_update_extended_selection (list, focus_row);
	      break;
	    }

	  if (list->anchor < 0)
	    gtk_hlist_set_anchor (list, FALSE, focus_row,
				 list->last_focus_child);
	  else
	    gtk_hlist_update_extended_selection (list, focus_row);
	  break;
	  
	default:
	  break;
	}
    }

  return FALSE;
}

static gint
gtk_hlist_button_release (GtkWidget	*widget,
			 GdkEventButton *event)
{
  GtkHList *list;
  GtkWidget *item;

  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_HLIST (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  list = GTK_HLIST (widget);

  /* we don't handle button 2 and 3 */
  if (event->button != 1)
    return FALSE;

  if (list->drag_selection)
    {
      gtk_hlist_end_drag_selection (list);

      switch (list->selection_mode)
	{
	case GTK_SELECTION_EXTENDED:
 	  if (!(event->state & GDK_SHIFT_MASK))
	    gtk_hlist_end_selection (list);
	  break;

	case GTK_SELECTION_SINGLE:
	case GTK_SELECTION_MULTIPLE:

	  item = gtk_get_event_widget ((GdkEvent*) event);
  
	  while (item && !GTK_IS_LIST_ITEM (item))
	    item = item->parent;
	  
	  if (item && item->parent == widget)
	    {
	      if (list->undo_focus_child == item)
		gtk_hlist_toggle_row (list, item);
	    }
	  list->undo_focus_child = NULL;
	  break;

	default:
	  break;
	}
    }
  
  return FALSE;
}

static void
gtk_hlist_draw (GtkWidget    *widget,
	       GdkRectangle *area)
{
  GtkHList *list;
  GtkWidget *child;
  GdkRectangle child_area;
  GList *children;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_HLIST (widget));
  g_return_if_fail (area != NULL);

  if (GTK_WIDGET_DRAWABLE (widget))
    {
      list = GTK_HLIST (widget);

      children = list->children;
      while (children)
	{
	  child = children->data;
	  children = children->next;

	  if (gtk_widget_intersect (child, area, &child_area))
	    gtk_widget_draw (child, &child_area);
	}
    }
}

static gint
gtk_hlist_expose (GtkWidget	*widget,
		 GdkEventExpose *event)
{
  GtkHList *list;
  GtkWidget *child;
  GdkEventExpose child_event;
  GList *children;

  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_HLIST (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  if (GTK_WIDGET_DRAWABLE (widget))
    {
      list = GTK_HLIST (widget);

      child_event = *event;

      children = list->children;
      while (children)
	{
	  child = children->data;
	  children = children->next;

	  if (GTK_WIDGET_NO_WINDOW (child) &&
	      gtk_widget_intersect (child, &event->area, &child_event.area))
	    gtk_widget_event (child, (GdkEvent*) &child_event);
	}
    }

  return FALSE;
}

static void 
gtk_hlist_style_set	(GtkWidget      *widget,
			 GtkStyle       *previous_style)
{
  g_return_if_fail (widget != NULL);

  if (previous_style && GTK_WIDGET_REALIZED (widget))
    gdk_window_set_background (widget->window, &widget->style->base[GTK_WIDGET_STATE (widget)]);
}

/* GtkContainer Methods :
 * gtk_hlist_add
 * gtk_hlist_remove
 * gtk_hlist_forall
 * gtk_hlist_child_type
 * gtk_hlist_set_focus_child
 * gtk_hlist_focus
 */
static void
gtk_hlist_add (GtkContainer *container,
	      GtkWidget	   *widget)
{
  GList *item_list;

  g_return_if_fail (container != NULL);
  g_return_if_fail (GTK_IS_HLIST (container));
  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_LIST_ITEM (widget));

  item_list = g_list_alloc ();
  item_list->data = widget;
  
  gtk_hlist_append_items (GTK_HLIST (container), item_list);
}

static void
gtk_hlist_remove (GtkContainer *container,
		 GtkWidget    *widget)
{
  GList *item_list;
  
  g_return_if_fail (container != NULL);
  g_return_if_fail (GTK_IS_HLIST (container));
  g_return_if_fail (widget != NULL);
  g_return_if_fail (container == GTK_CONTAINER (widget->parent));
  
  item_list = g_list_alloc ();
  item_list->data = widget;
  
  gtk_hlist_remove_items (GTK_HLIST (container), item_list);
  
  g_list_free (item_list);
}

static void
gtk_hlist_forall (GtkContainer  *container,
		 gboolean       include_internals,
		 GtkCallback	callback,
		 gpointer	callback_data)
{
  GtkHList *list;
  GtkWidget *child;
  GList *children;

  g_return_if_fail (container != NULL);
  g_return_if_fail (GTK_IS_HLIST (container));
  g_return_if_fail (callback != NULL);

  list = GTK_HLIST (container);
  children = list->children;

  while (children)
    {
      child = children->data;
      children = children->next;

      (* callback) (child, callback_data);
    }
}

static GtkType
gtk_hlist_child_type (GtkContainer *container)
{
  return GTK_TYPE_LIST_ITEM;
}

static void
gtk_hlist_set_focus_child (GtkContainer *container,
			  GtkWidget    *child)
{
  GtkHList *list;

  g_return_if_fail (container != NULL);
  g_return_if_fail (GTK_IS_HLIST (container));
 
  if (child)
    g_return_if_fail (GTK_IS_WIDGET (child));

  list = GTK_HLIST (container);

  if (child != container->focus_child)
    {
      if (container->focus_child)
	{
	  list->last_focus_child = container->focus_child;
	  gtk_widget_unref (container->focus_child);
	}
      container->focus_child = child;
      if (container->focus_child)
        gtk_widget_ref (container->focus_child);
    }

  /* check for v adjustment */
  if (container->focus_child)
    {
      GtkAdjustment *adjustment;

      adjustment = gtk_object_get_data_by_id (GTK_OBJECT (container),
					      vadjustment_key_id);
      if (adjustment)
        gtk_adjustment_clamp_page (adjustment,
                                   container->focus_child->allocation.y,
                                   (container->focus_child->allocation.y +
                                    container->focus_child->allocation.height));
      switch (list->selection_mode)
	{
	case GTK_SELECTION_BROWSE:
	  gtk_hlist_select_child (list, child);
	  break;
	case GTK_SELECTION_EXTENDED:
	  if (!list->last_focus_child && !list->add_mode)
	    {
	      list->undo_focus_child = list->last_focus_child;
	      gtk_hlist_unselect_all (list);
	      gtk_hlist_select_child (list, child);
	    }
	  break;
	default:
	  break;
	}
    }
}

static gint
gtk_hlist_focus (GtkContainer     *container,
		GtkDirectionType  direction)
{
  gint return_val = FALSE;

  g_return_val_if_fail (container != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_HLIST (container), FALSE);

  if (container->focus_child == NULL ||
      !GTK_WIDGET_HAS_FOCUS (container->focus_child))
    {
      if (GTK_HLIST (container)->last_focus_child)
	gtk_container_set_focus_child
	  (container, GTK_HLIST (container)->last_focus_child);

      if (GTK_CONTAINER_CLASS (parent_class)->focus)
	return_val = GTK_CONTAINER_CLASS (parent_class)->focus (container,
								direction);
    }

  if (!return_val)
    {
      GtkHList *list;

      list =  GTK_HLIST (container);
      if (list->selection_mode == GTK_SELECTION_EXTENDED && list->anchor >= 0)
	gtk_hlist_end_selection (list);

      if (container->focus_child)
	list->last_focus_child = container->focus_child;
    }

  return return_val;
}


/* Public GtkHList Methods :
 *
 * gtk_hlist_insert_items
 * gtk_hlist_append_items
 * gtk_hlist_prepend_items
 * gtk_hlist_remove_items
 * gtk_hlist_remove_items_no_unref
 * gtk_hlist_clear_items
 *
 * gtk_hlist_child_position
 */
void
gtk_hlist_insert_items (GtkHList *list,
		       GList   *items,
		       gint	position)
{
  GtkWidget *widget;
  GList *tmp_list;
  GList *last;
  gint nchildren;

  g_return_if_fail (list != NULL);
  g_return_if_fail (GTK_IS_HLIST (list));

  if (!items)
    return;

  gtk_hlist_end_drag_selection (list);
  if (list->selection_mode == GTK_SELECTION_EXTENDED && list->anchor >= 0)
    gtk_hlist_end_selection (list);

  tmp_list = items;
  while (tmp_list)
    {
      widget = tmp_list->data;
      tmp_list = tmp_list->next;

      gtk_widget_set_parent (widget, GTK_WIDGET (list));
      gtk_signal_connect (GTK_OBJECT (widget), "drag_begin",
			  GTK_SIGNAL_FUNC (gtk_hlist_signal_drag_begin),
			  list);
      gtk_signal_connect (GTK_OBJECT (widget), "toggle_focus_row",
			  GTK_SIGNAL_FUNC (gtk_hlist_signal_toggle_focus_row),
			  list);
      gtk_signal_connect (GTK_OBJECT (widget), "select_all",
			  GTK_SIGNAL_FUNC (gtk_hlist_signal_select_all),
			  list);
      gtk_signal_connect (GTK_OBJECT (widget), "unselect_all",
			  GTK_SIGNAL_FUNC (gtk_hlist_signal_unselect_all),
			  list);
      gtk_signal_connect (GTK_OBJECT (widget), "undo_selection",
			  GTK_SIGNAL_FUNC (gtk_hlist_signal_undo_selection),
			  list);
      gtk_signal_connect (GTK_OBJECT (widget), "start_selection",
			  GTK_SIGNAL_FUNC (gtk_hlist_signal_start_selection),
			  list);
      gtk_signal_connect (GTK_OBJECT (widget), "end_selection",
			  GTK_SIGNAL_FUNC (gtk_hlist_signal_end_selection),
			  list);
      gtk_signal_connect (GTK_OBJECT (widget), "extend_selection",
			  GTK_SIGNAL_FUNC (gtk_hlist_signal_extend_selection),
			  list);
      gtk_signal_connect (GTK_OBJECT (widget), "scroll_horizontal",
			  GTK_SIGNAL_FUNC (gtk_hlist_signal_scroll_horizontal),
			  list);
      gtk_signal_connect (GTK_OBJECT (widget), "scroll_vertical",
			  GTK_SIGNAL_FUNC (gtk_hlist_signal_scroll_vertical),
			  list);
      gtk_signal_connect (GTK_OBJECT (widget), "toggle_add_mode",
			  GTK_SIGNAL_FUNC (gtk_hlist_signal_toggle_add_mode),
			  list);
      gtk_signal_connect (GTK_OBJECT (widget), "select",
			  GTK_SIGNAL_FUNC (gtk_hlist_signal_item_select),
			  list);
      gtk_signal_connect (GTK_OBJECT (widget), "deselect",
			  GTK_SIGNAL_FUNC (gtk_hlist_signal_item_deselect),
			  list);
      gtk_signal_connect (GTK_OBJECT (widget), "toggle",
			  GTK_SIGNAL_FUNC (gtk_hlist_signal_item_toggle),
			  list);

      if (GTK_WIDGET_REALIZED (widget->parent))
	gtk_widget_realize (widget);

      if (GTK_WIDGET_VISIBLE (widget->parent) && GTK_WIDGET_VISIBLE (widget))
	{
	  if (GTK_WIDGET_MAPPED (widget->parent))
	    gtk_widget_map (widget);

	  gtk_widget_queue_resize (widget);
	}
    }


  nchildren = g_list_length (list->children);
  if ((position < 0) || (position > nchildren))
    position = nchildren;

  if (position == nchildren)
    {
      if (list->children)
	{
	  tmp_list = g_list_last (list->children);
	  tmp_list->next = items;
	  items->prev = tmp_list;
	}
      else
	{
	  list->children = items;
	}
    }
  else
    {
      tmp_list = g_list_nth (list->children, position);
      last = g_list_last (items);

      if (tmp_list->prev)
	tmp_list->prev->next = items;
      last->next = tmp_list;
      items->prev = tmp_list->prev;
      tmp_list->prev = last;

      if (tmp_list == list->children)
	list->children = items;
    }
  
  if (list->children && !list->selection &&
      (list->selection_mode == GTK_SELECTION_BROWSE))
    {
      widget = list->children->data;
      gtk_hlist_select_child (list, widget);
    }
}

void
gtk_hlist_append_items (GtkHList *list,
		       GList   *items)
{
  g_return_if_fail (list != NULL);
  g_return_if_fail (GTK_IS_HLIST (list));

  gtk_hlist_insert_items (list, items, -1);
}

void
gtk_hlist_prepend_items (GtkHList *list,
			GList	*items)
{
  g_return_if_fail (list != NULL);
  g_return_if_fail (GTK_IS_HLIST (list));

  gtk_hlist_insert_items (list, items, 0);
}

void
gtk_hlist_remove_items (GtkHList	*list,
		       GList	*items)
{
  gtk_hlist_remove_items_internal (list, items, FALSE);
}

void
gtk_hlist_remove_items_no_unref (GtkHList	 *list,
				GList	 *items)
{
  gtk_hlist_remove_items_internal (list, items, TRUE);
}

void
gtk_hlist_clear_items (GtkHList *list,
		      gint     start,
		      gint     end)
{
  GtkContainer *container;
  GtkWidget *widget;
  GtkWidget *new_focus_child = NULL;
  GList *start_list;
  GList *end_list;
  GList *tmp_list;
  guint nchildren;
  gboolean grab_focus = FALSE;

  g_return_if_fail (list != NULL);
  g_return_if_fail (GTK_IS_HLIST (list));

  nchildren = g_list_length (list->children);

  if (nchildren == 0)
    return;

  if ((end < 0) || (end > nchildren))
    end = nchildren;

  if (start >= end)
    return;

  container = GTK_CONTAINER (list);

  gtk_hlist_end_drag_selection (list);
  if (list->selection_mode == GTK_SELECTION_EXTENDED)
    {
      if (list->anchor >= 0)
	gtk_hlist_end_selection (list);

      gtk_hlist_reset_extended_selection (list);
    }

  start_list = g_list_nth (list->children, start);
  end_list = g_list_nth (list->children, end);

  if (start_list->prev)
    start_list->prev->next = end_list;
  if (end_list && end_list->prev)
    end_list->prev->next = NULL;
  if (end_list)
    end_list->prev = start_list->prev;
  if (start_list == list->children)
    list->children = end_list;

  if (container->focus_child)
    {
      if (g_list_find (start_list, container->focus_child))
	{
	  if (start_list->prev)
	    new_focus_child = start_list->prev->data;
	  else if (list->children)
	    new_focus_child = list->children->prev->data;

	  if (GTK_WIDGET_HAS_FOCUS (container->focus_child))
	    grab_focus = TRUE;
	}
    }

  tmp_list = start_list;
  while (tmp_list)
    {
      widget = tmp_list->data;
      tmp_list = tmp_list->next;

      if (widget->state == GTK_STATE_SELECTED)
	gtk_hlist_unselect_child (list, widget);

      if (widget == list->undo_focus_child)
	list->undo_focus_child = NULL;
      if (widget == list->last_focus_child)
	list->last_focus_child = NULL;

      gtk_signal_disconnect_by_data (GTK_OBJECT (widget), (gpointer) list);
      gtk_widget_unparent (widget);
    }

  g_list_free (start_list);

  if (new_focus_child)
    {
      if (grab_focus)
	gtk_widget_grab_focus (new_focus_child);
      else if (container->focus_child)
	gtk_container_set_focus_child (container, new_focus_child);

      if ((list->selection_mode == GTK_SELECTION_BROWSE ||
	   list->selection_mode == GTK_SELECTION_EXTENDED) && !list->selection)
	{
	  list->last_focus_child = new_focus_child; 
	  gtk_hlist_select_child (list, new_focus_child);
	}
    }

  if (GTK_WIDGET_VISIBLE (list))
    gtk_widget_queue_resize (GTK_WIDGET (list));
}

gint
gtk_hlist_child_position (GtkHList   *list,
			 GtkWidget *child)
{
  GList *children;
  gint pos;

  g_return_val_if_fail (list != NULL, -1);
  g_return_val_if_fail (GTK_IS_HLIST (list), -1);
  g_return_val_if_fail (child != NULL, -1);

  pos = 0;
  children = list->children;

  while (children)
    {
      if (child == GTK_WIDGET (children->data))
	return pos;

      pos += 1;
      children = children->next;
    }

  return -1;
}


/* Private GtkHList Insert/Remove Item Functions:
 *
 * gtk_hlist_remove_items_internal
 */
static void
gtk_hlist_remove_items_internal (GtkHList	 *list,
				GList	 *items,
				gboolean  no_unref)
{
  GtkWidget *widget;
  GtkWidget *new_focus_child;
  GtkWidget *old_focus_child;
  GtkContainer *container;
  GList *tmp_list;
  GList *work;
  gboolean grab_focus = FALSE;
  
  g_return_if_fail (list != NULL);
  g_return_if_fail (GTK_IS_HLIST (list));

  if (!items)
    return;
  
  container = GTK_CONTAINER (list);

  gtk_hlist_end_drag_selection (list);
  if (list->selection_mode == GTK_SELECTION_EXTENDED)
    {
      if (list->anchor >= 0)
	gtk_hlist_end_selection (list);

      gtk_hlist_reset_extended_selection (list);
    }

  tmp_list = items;
  while (tmp_list)
    {
      widget = tmp_list->data;
      tmp_list = tmp_list->next;
      
      if (widget->state == GTK_STATE_SELECTED)
	gtk_hlist_unselect_child (list, widget);
    }

  if (container->focus_child)
    {
      old_focus_child = new_focus_child = container->focus_child;
      if (GTK_WIDGET_HAS_FOCUS (container->focus_child))
	grab_focus = TRUE;
    }
  else
    old_focus_child = new_focus_child = list->last_focus_child;

  tmp_list = items;
  while (tmp_list)
    {
      widget = tmp_list->data;
      tmp_list = tmp_list->next;

      if (no_unref)
	gtk_widget_ref (widget);

      if (widget == new_focus_child) 
	{
	  work = g_list_find (list->children, widget);

	  if (work)
	    {
	      if (work->next)
		new_focus_child = work->next->data;
	      else if (list->children != work && work->prev)
		new_focus_child = work->prev->data;
	      else
		new_focus_child = NULL;
	    }
	}

      if (widget == list->undo_focus_child)
	list->undo_focus_child = NULL;
      if (widget == list->last_focus_child)
	list->last_focus_child = NULL;

      gtk_signal_disconnect_by_data (GTK_OBJECT (widget), (gpointer) list);
      list->children = g_list_remove (list->children, widget);
      gtk_widget_unparent (widget);
    }
  
  if (new_focus_child && new_focus_child != old_focus_child)
    {
      if (grab_focus)
	gtk_widget_grab_focus (new_focus_child);
      else if (container->focus_child)
	gtk_container_set_focus_child (container, new_focus_child);

      if (list->selection_mode == GTK_SELECTION_BROWSE && !list->selection)
	{
	  list->last_focus_child = new_focus_child; 
	  gtk_hlist_select_child (list, new_focus_child);
	}
    }

  if (GTK_WIDGET_VISIBLE (list))
    gtk_widget_queue_resize (GTK_WIDGET (list));
}


/* Public GtkHList Selection Methods :
 *
 * gtk_hlist_set_selection_mode
 * gtk_hlist_select_item
 * gtk_hlist_unselect_item
 * gtk_hlist_select_child
 * gtk_hlist_unselect_child
 * gtk_hlist_select_all
 * gtk_hlist_unselect_all
 * gtk_hlist_extend_selection
 * gtk_hlist_end_drag_selection
 * gtk_hlist_start_selection
 * gtk_hlist_end_selection
 * gtk_hlist_toggle_row
 * gtk_hlist_toggle_focus_row
 * gtk_hlist_toggle_add_mode
 * gtk_hlist_undo_selection
 */
void
gtk_hlist_set_selection_mode (GtkHList	      *list,
			     GtkSelectionMode  mode)
{
  g_return_if_fail (list != NULL);
  g_return_if_fail (GTK_IS_HLIST (list));

  if (list->selection_mode == mode)
    return;

  list->selection_mode = mode;

  switch (mode)
    {
    case GTK_SELECTION_SINGLE:
    case GTK_SELECTION_BROWSE:
      gtk_hlist_unselect_all (list);
      break;
    default:
      break;
    }
}

void
gtk_hlist_select_item (GtkHList *list,
		      gint     item)
{
  GList *tmp_list;

  g_return_if_fail (list != NULL);
  g_return_if_fail (GTK_IS_HLIST (list));

  tmp_list = g_list_nth (list->children, item);
  if (tmp_list)
    gtk_hlist_select_child (list, GTK_WIDGET (tmp_list->data));
}

void
gtk_hlist_unselect_item (GtkHList *list,
			gint	 item)
{
  GList *tmp_list;

  g_return_if_fail (list != NULL);
  g_return_if_fail (GTK_IS_HLIST (list));

  tmp_list = g_list_nth (list->children, item);
  if (tmp_list)
    gtk_hlist_unselect_child (list, GTK_WIDGET (tmp_list->data));
}

void
gtk_hlist_select_child (GtkHList	 *list,
		       GtkWidget *child)
{
  gtk_signal_emit (GTK_OBJECT (list), list_signals[SELECT_CHILD], child);
}

void
gtk_hlist_unselect_child (GtkHList   *list,
			 GtkWidget *child)
{
  gtk_signal_emit (GTK_OBJECT (list), list_signals[UNSELECT_CHILD], child);
}

void
gtk_hlist_select_all (GtkHList *list)
{
  GtkContainer *container;
  GList *work;
 
  g_return_if_fail (list != NULL);
  g_return_if_fail (GTK_IS_HLIST (list));

  if (!list->children)
    return;
  
  if (gdk_pointer_is_grabbed () && GTK_WIDGET_HAS_GRAB (list))
    gtk_hlist_end_drag_selection (list);

  if (list->selection_mode == GTK_SELECTION_EXTENDED && list->anchor >= 0)
    gtk_hlist_end_selection (list);

  container = GTK_CONTAINER (list);

  switch (list->selection_mode)
    {
    case GTK_SELECTION_BROWSE:
      if (container->focus_child)
	{
	  gtk_hlist_select_child (list, container->focus_child);
	  return;
	}
      break;
    case GTK_SELECTION_EXTENDED:
      g_list_free (list->undo_selection);
      g_list_free (list->undo_unselection);
      list->undo_selection = NULL;
      list->undo_unselection = NULL;

      if (list->children &&
	  GTK_WIDGET_STATE (list->children->data) != GTK_STATE_SELECTED)
	gtk_hlist_fake_toggle_row (list, GTK_WIDGET (list->children->data));

      list->anchor_state =  GTK_STATE_SELECTED;
      list->anchor = 0;
      list->drag_pos = 0;
      list->undo_focus_child = container->focus_child;
      gtk_hlist_update_extended_selection (list, g_list_length(list->children));
      gtk_hlist_end_selection (list);
      return;
    case GTK_SELECTION_MULTIPLE:
      for (work = list->children; work; work = work->next)
	{
	  if (GTK_WIDGET_STATE (work->data) == GTK_STATE_NORMAL)
	    gtk_hlist_select_child (list, GTK_WIDGET (work->data));
	}
      return;
    default:
      break;
    }
}

void
gtk_hlist_unselect_all (GtkHList *list)
{
  GtkContainer *container;
  GtkWidget *item;
  GList *work;
 
  g_return_if_fail (list != NULL);
  g_return_if_fail (GTK_IS_HLIST (list));

  if (!list->children)
    return;

  if (gdk_pointer_is_grabbed () && GTK_WIDGET_HAS_GRAB (list))
    gtk_hlist_end_drag_selection (list);

  if (list->selection_mode == GTK_SELECTION_EXTENDED && list->anchor >= 0)
    gtk_hlist_end_selection (list);

  container = GTK_CONTAINER (list);

  switch (list->selection_mode)
    {
    case GTK_SELECTION_BROWSE:
      if (container->focus_child)
	{
	  gtk_hlist_select_child (list, container->focus_child);
	  return;
	}
      break;
    case GTK_SELECTION_EXTENDED:
      gtk_hlist_reset_extended_selection (list);
      break;
    default:
      break;
    }

  work = list->selection;

  while (work)
    {
      item = work->data;
      work = work->next;
      gtk_hlist_unselect_child (list, item);
    }
}

void
gtk_hlist_extend_selection (GtkHList       *list,
			   GtkScrollType  scroll_type,
			   gfloat         position,
			   gboolean       auto_start_selection)
{
  GtkContainer *container;

  g_return_if_fail (list != NULL);
  g_return_if_fail (GTK_IS_HLIST (list));

  if ((gdk_pointer_is_grabbed () && GTK_WIDGET_HAS_GRAB (list)) ||
      list->selection_mode != GTK_SELECTION_EXTENDED)
    return;

  container = GTK_CONTAINER (list);

  if (auto_start_selection)
    {
      gint focus_row;

      focus_row = g_list_index (list->children, container->focus_child);
      gtk_hlist_set_anchor (list, list->add_mode, focus_row,
			   container->focus_child);
    }
  else if (list->anchor < 0)
    return;

  gtk_hlist_move_focus_child (list, scroll_type, position);
  gtk_hlist_update_extended_selection 
    (list, g_list_index (list->children, container->focus_child));
}

void
gtk_hlist_end_drag_selection (GtkHList *list)
{
  g_return_if_fail (list != NULL);
  g_return_if_fail (GTK_IS_HLIST (list));

  list->drag_selection = FALSE;
  if (GTK_WIDGET_HAS_GRAB (list))
    {
      gtk_grab_remove (GTK_WIDGET (list));
      if (gdk_pointer_is_grabbed())
	gdk_pointer_ungrab (GDK_CURRENT_TIME);
    }
  if (list->htimer)
    {
      gtk_timeout_remove (list->htimer);
      list->htimer = 0;
    }
  if (list->vtimer)
    {
      gtk_timeout_remove (list->vtimer);
      list->vtimer = 0;
    }
}

void
gtk_hlist_start_selection (GtkHList *list)
{
  GtkContainer *container;
  gint focus_row;

  g_return_if_fail (list != NULL);
  g_return_if_fail (GTK_IS_HLIST (list));

  if (gdk_pointer_is_grabbed () && GTK_WIDGET_HAS_GRAB (list))
    return;

  container = GTK_CONTAINER (list);

  if ((focus_row = g_list_index (list->selection, container->focus_child))
      >= 0)
    gtk_hlist_set_anchor (list, list->add_mode,
			 focus_row, container->focus_child);
}

void
gtk_hlist_end_selection (GtkHList *list)
{
  gint i;
  gint e;
  gboolean top_down;
  GList *work;
  GtkWidget *item;
  gint item_index;

  g_return_if_fail (list != NULL);
  g_return_if_fail (GTK_IS_HLIST (list));

  if ((gdk_pointer_is_grabbed () && GTK_WIDGET_HAS_GRAB (list)) ||
      list->anchor < 0)
    return;

  i = MIN (list->anchor, list->drag_pos);
  e = MAX (list->anchor, list->drag_pos);

  top_down = (list->anchor < list->drag_pos);

  list->anchor = -1;
  list->drag_pos = -1;
  
  if (list->undo_selection)
    {
      work = list->selection;
      list->selection = list->undo_selection;
      list->undo_selection = work;
      work = list->selection;
      while (work)
	{
	  item = work->data;
	  work = work->next;
	  item_index = g_list_index (list->children, item);
	  if (item_index < i || item_index > e)
	    {
	      gtk_widget_set_state (item, GTK_STATE_SELECTED);
	      gtk_hlist_unselect_child (list, item);
	      list->undo_selection = g_list_prepend (list->undo_selection,
						     item);
	    }
	}
    }    

  if (top_down)
    {
      for (work = g_list_nth (list->children, i); i <= e;
	   i++, work = work->next)
	{
	  item = work->data;
	  if (g_list_find (list->selection, item))
	    {
	      if (item->state == GTK_STATE_NORMAL)
		{
		  gtk_widget_set_state (item, GTK_STATE_SELECTED);
		  gtk_hlist_unselect_child (list, item);
		  list->undo_selection = g_list_prepend (list->undo_selection,
							 item);
		}
	    }
	  else if (item->state == GTK_STATE_SELECTED)
	    {
	      gtk_widget_set_state (item, GTK_STATE_NORMAL);
	      list->undo_unselection = g_list_prepend (list->undo_unselection,
						       item);
	    }
	}
    }
  else
    {
      for (work = g_list_nth (list->children, e); i <= e;
	   e--, work = work->prev)
	{
	  item = work->data;
	  if (g_list_find (list->selection, item))
	    {
	      if (item->state == GTK_STATE_NORMAL)
		{
		  gtk_widget_set_state (item, GTK_STATE_SELECTED);
		  gtk_hlist_unselect_child (list, item);
		  list->undo_selection = g_list_prepend (list->undo_selection,
							 item);
		}
	    }
	  else if (item->state == GTK_STATE_SELECTED)
	    {
	      gtk_widget_set_state (item, GTK_STATE_NORMAL);
	      list->undo_unselection = g_list_prepend (list->undo_unselection,
						       item);
	    }
	}
    }

  for (work = g_list_reverse (list->undo_unselection); work; work = work->next)
    gtk_hlist_select_child (list, GTK_WIDGET (work->data));


}

void
gtk_hlist_toggle_row (GtkHList   *list,
		     GtkWidget *item)
{
  g_return_if_fail (list != NULL);
  g_return_if_fail (GTK_IS_HLIST (list));
  g_return_if_fail (item != NULL);
  g_return_if_fail (GTK_IS_LIST_ITEM (item));

  switch (list->selection_mode)
    {
    case GTK_SELECTION_EXTENDED:
    case GTK_SELECTION_MULTIPLE:
    case GTK_SELECTION_SINGLE:
      if (item->state == GTK_STATE_SELECTED)
	{
	  gtk_hlist_unselect_child (list, item);
	  return;
	}
    case GTK_SELECTION_BROWSE:
      gtk_hlist_select_child (list, item);
      break;
    }
}

void
gtk_hlist_toggle_focus_row (GtkHList *list)
{
  GtkContainer *container;
  gint focus_row;

  g_return_if_fail (list != 0);
  g_return_if_fail (GTK_IS_HLIST (list));

  container = GTK_CONTAINER (list);

  if ((gdk_pointer_is_grabbed () && GTK_WIDGET_HAS_GRAB (list)) ||
      !container->focus_child)
    return;

  switch (list->selection_mode)
    {
    case  GTK_SELECTION_SINGLE:
    case  GTK_SELECTION_MULTIPLE:
      gtk_hlist_toggle_row (list, container->focus_child);
      break;
    case GTK_SELECTION_EXTENDED:
      if ((focus_row = g_list_index (list->children, container->focus_child))
	  < 0)
	return;

      g_list_free (list->undo_selection);
      g_list_free (list->undo_unselection);
      list->undo_selection = NULL;
      list->undo_unselection = NULL;

      list->anchor = focus_row;
      list->drag_pos = focus_row;
      list->undo_focus_child = container->focus_child;

      if (list->add_mode)
	gtk_hlist_fake_toggle_row (list, container->focus_child);
      else
	gtk_hlist_fake_unselect_all (list, container->focus_child);
      
      gtk_hlist_end_selection (list);
      break;
    default:
      break;
    }
}

void
gtk_hlist_toggle_add_mode (GtkHList *list)
{
  GtkContainer *container;

  g_return_if_fail (list != 0);
  g_return_if_fail (GTK_IS_HLIST (list));
  
  if ((gdk_pointer_is_grabbed () && GTK_WIDGET_HAS_GRAB (list)) ||
      list->selection_mode != GTK_SELECTION_EXTENDED)
    return;
  
  container = GTK_CONTAINER (list);

  if (list->add_mode)
    {
      list->add_mode = FALSE;
      list->anchor_state = GTK_STATE_SELECTED;
    }
  else
    list->add_mode = TRUE;
  
  if (container->focus_child)
    gtk_widget_queue_draw (container->focus_child);
}

void
gtk_hlist_undo_selection (GtkHList *list)
{
  GList *work;

  g_return_if_fail (list != NULL);
  g_return_if_fail (GTK_IS_HLIST (list));

  if (list->selection_mode != GTK_SELECTION_EXTENDED ||
      (gdk_pointer_is_grabbed () && GTK_WIDGET_HAS_GRAB (list)))
    return;
  
  if (list->anchor >= 0)
    gtk_hlist_end_selection (list);

  if (!(list->undo_selection || list->undo_unselection))
    {
      gtk_hlist_unselect_all (list);
      return;
    }

  for (work = list->undo_selection; work; work = work->next)
    gtk_hlist_select_child (list, GTK_WIDGET (work->data));

  for (work = list->undo_unselection; work; work = work->next)
    gtk_hlist_unselect_child (list, GTK_WIDGET (work->data));

  if (list->undo_focus_child)
    {
      GtkContainer *container;

      container = GTK_CONTAINER (list);

      if (container->focus_child &&
	  GTK_WIDGET_HAS_FOCUS (container->focus_child))
	gtk_widget_grab_focus (list->undo_focus_child);
      else
	gtk_container_set_focus_child (container, list->undo_focus_child);
    }

  list->undo_focus_child = NULL;
 
  g_list_free (list->undo_selection);
  g_list_free (list->undo_unselection);
  list->undo_selection = NULL;
  list->undo_unselection = NULL;
}


/* Private GtkHList Selection Methods :
 *
 * gtk_real_list_select_child
 * gtk_real_list_unselect_child
 */
static void
gtk_real_list_select_child (GtkHList   *list,
			    GtkWidget *child)
{
  g_return_if_fail (list != NULL);
  g_return_if_fail (GTK_IS_HLIST (list));
  g_return_if_fail (child != NULL);
  g_return_if_fail (GTK_IS_LIST_ITEM (child));

  switch (child->state)
    {
    case GTK_STATE_SELECTED:
    case GTK_STATE_INSENSITIVE:
      break;
    default:
      gtk_list_item_select (GTK_LIST_ITEM (child));
      break;
    }
}

static void
gtk_real_list_unselect_child (GtkHList	*list,
			      GtkWidget *child)
{
  g_return_if_fail (list != NULL);
  g_return_if_fail (GTK_IS_HLIST (list));
  g_return_if_fail (child != NULL);
  g_return_if_fail (GTK_IS_LIST_ITEM (child));

  if (child->state == GTK_STATE_SELECTED)
    gtk_list_item_deselect (GTK_LIST_ITEM (child));
}


/* Private GtkHList Selection Functions :
 *
 * gtk_hlist_set_anchor
 * gtk_hlist_fake_unselect_all
 * gtk_hlist_fake_toggle_row
 * gtk_hlist_update_extended_selection
 * gtk_hlist_reset_extended_selection
 */
static void
gtk_hlist_set_anchor (GtkHList   *list,
		     gboolean   add_mode,
		     gint       anchor,
		     GtkWidget *undo_focus_child)
{
  GList *work;

  g_return_if_fail (list != NULL);
  g_return_if_fail (GTK_IS_HLIST (list));
  
  if (list->selection_mode != GTK_SELECTION_EXTENDED || list->anchor >= 0)
    return;

  g_list_free (list->undo_selection);
  g_list_free (list->undo_unselection);
  list->undo_selection = NULL;
  list->undo_unselection = NULL;

  if ((work = g_list_nth (list->children, anchor)))
    {
      if (add_mode)
	gtk_hlist_fake_toggle_row (list, GTK_WIDGET (work->data));
      else
	{
	  gtk_hlist_fake_unselect_all (list, GTK_WIDGET (work->data));
	  list->anchor_state = GTK_STATE_SELECTED;
	}
    }

  list->anchor = anchor;
  list->drag_pos = anchor;
  list->undo_focus_child = undo_focus_child;
}

static void
gtk_hlist_fake_unselect_all (GtkHList   *list,
			    GtkWidget *item)
{
  GList *work;

  if (item && item->state == GTK_STATE_NORMAL)
    gtk_widget_set_state (item, GTK_STATE_SELECTED);

  list->undo_selection = list->selection;
  list->selection = NULL;
  
  for (work = list->undo_selection; work; work = work->next)
    if (work->data != item)
      gtk_widget_set_state (GTK_WIDGET (work->data), GTK_STATE_NORMAL);
}

static void
gtk_hlist_fake_toggle_row (GtkHList   *list,
			  GtkWidget *item)
{
  if (!item)
    return;
  
  if (item->state == GTK_STATE_NORMAL)
    {
      list->anchor_state = GTK_STATE_SELECTED;
      gtk_widget_set_state (item, GTK_STATE_SELECTED);
    }
  else
    {
      list->anchor_state = GTK_STATE_NORMAL;
      gtk_widget_set_state (item, GTK_STATE_NORMAL);
    }
}

static void
gtk_hlist_update_extended_selection (GtkHList *list,
				    gint     row)
{
  gint i;
  GList *work;
  gint s1 = -1;
  gint s2 = -1;
  gint e1 = -1;
  gint e2 = -1;
  gint length;

  if (row < 0)
    row = 0;

  length = g_list_length (list->children);
  if (row >= length)
    row = length - 1;

  if (list->selection_mode != GTK_SELECTION_EXTENDED || !list->anchor < 0)
    return;

  /* extending downwards */
  if (row > list->drag_pos && list->anchor <= list->drag_pos)
    {
      s2 = list->drag_pos + 1;
      e2 = row;
    }
  /* extending upwards */
  else if (row < list->drag_pos && list->anchor >= list->drag_pos)
    {
      s2 = row;
      e2 = list->drag_pos - 1;
    }
  else if (row < list->drag_pos && list->anchor < list->drag_pos)
    {
      e1 = list->drag_pos;
      /* row and drag_pos on different sides of anchor :
	 take back the selection between anchor and drag_pos,
         select between anchor and row */
      if (row < list->anchor)
	{
	  s1 = list->anchor + 1;
	  s2 = row;
	  e2 = list->anchor - 1;
	}
      /* take back the selection between anchor and drag_pos */
      else
	s1 = row + 1;
    }
  else if (row > list->drag_pos && list->anchor > list->drag_pos)
    {
      s1 = list->drag_pos;
      /* row and drag_pos on different sides of anchor :
	 take back the selection between anchor and drag_pos,
         select between anchor and row */
      if (row > list->anchor)
	{
	  e1 = list->anchor - 1;
	  s2 = list->anchor + 1;
	  e2 = row;
	}
      /* take back the selection between anchor and drag_pos */
      else
	e1 = row - 1;
    }

  list->drag_pos = row;

  /* restore the elements between s1 and e1 */
  if (s1 >= 0)
    {
      for (i = s1, work = g_list_nth (list->children, i); i <= e1;
	   i++, work = work->next)
	{
	  if (g_list_find (list->selection, work->data))
            gtk_widget_set_state (GTK_WIDGET (work->data), GTK_STATE_SELECTED);
          else
            gtk_widget_set_state (GTK_WIDGET (work->data), GTK_STATE_NORMAL);
	}
    }

  /* extend the selection between s2 and e2 */
  if (s2 >= 0)
    {
      for (i = s2, work = g_list_nth (list->children, i); i <= e2;
	   i++, work = work->next)
	if (GTK_WIDGET (work->data)->state != list->anchor_state)
	  gtk_widget_set_state (GTK_WIDGET (work->data), list->anchor_state);
    }
}

static void
gtk_hlist_reset_extended_selection (GtkHList *list)
{ 
  g_return_if_fail (list != 0);
  g_return_if_fail (GTK_IS_HLIST (list));

  g_list_free (list->undo_selection);
  g_list_free (list->undo_unselection);
  list->undo_selection = NULL;
  list->undo_unselection = NULL;

  list->anchor = -1;
  list->drag_pos = -1;
  list->undo_focus_child = GTK_CONTAINER (list)->focus_child;
}

/* Public GtkHList Scroll Methods :
 *
 * gtk_hlist_scroll_horizontal
 * gtk_hlist_scroll_vertical
 */
void
gtk_hlist_scroll_horizontal (GtkHList       *list,
			    GtkScrollType  scroll_type,
			    gfloat         position)
{
  GtkAdjustment *adj;

  g_return_if_fail (list != 0);
  g_return_if_fail (GTK_IS_HLIST (list));

  if (gdk_pointer_is_grabbed () && GTK_WIDGET_HAS_GRAB (list))
    return;

  if (!(adj =
	gtk_object_get_data_by_id (GTK_OBJECT (list), hadjustment_key_id)))
    return;

  switch (scroll_type)
    {
    case GTK_SCROLL_STEP_BACKWARD:
      adj->value = CLAMP (adj->value - adj->step_increment, adj->lower,
			  adj->upper - adj->page_size);
      break;
    case GTK_SCROLL_STEP_FORWARD:
      adj->value = CLAMP (adj->value + adj->step_increment, adj->lower,
			  adj->upper - adj->page_size);
      break;
    case GTK_SCROLL_PAGE_BACKWARD:
      adj->value = CLAMP (adj->value - adj->page_increment, adj->lower,
			  adj->upper - adj->page_size);
      break;
    case GTK_SCROLL_PAGE_FORWARD:
      adj->value = CLAMP (adj->value + adj->page_increment, adj->lower,
			  adj->upper - adj->page_size);
      break;
    case GTK_SCROLL_JUMP:
      adj->value = CLAMP (adj->lower + (adj->upper - adj->lower) * position,
			  adj->lower, adj->upper - adj->page_size);
      break;
    default:
      break;
    }
  gtk_adjustment_value_changed (adj);
}

void
gtk_hlist_scroll_vertical (GtkHList       *list,
			  GtkScrollType  scroll_type,
			  gfloat         position)
{
  g_return_if_fail (list != NULL);
  g_return_if_fail (GTK_IS_HLIST (list));

  if (gdk_pointer_is_grabbed () && GTK_WIDGET_HAS_GRAB (list))
    return;

  if (list->selection_mode == GTK_SELECTION_EXTENDED)
    {
      GtkContainer *container;

      if (list->anchor >= 0)
	return;

      container = GTK_CONTAINER (list);
      list->undo_focus_child = container->focus_child;
      gtk_hlist_move_focus_child (list, scroll_type, position);
      if (container->focus_child != list->undo_focus_child && !list->add_mode)
	{
	  gtk_hlist_unselect_all (list);
	  gtk_hlist_select_child (list, container->focus_child);
	}
    }
  else
    gtk_hlist_move_focus_child (list, scroll_type, position);
}


/* Private GtkHList Scroll/Focus Functions :
 *
 * gtk_hlist_move_focus_child
 * gtk_hlist_horizontal_timeout
 * gtk_hlist_vertical_timeout
 */
static void
gtk_hlist_move_focus_child (GtkHList       *list,
			   GtkScrollType  scroll_type,
			   gfloat         position)
{
  GtkContainer *container;
  GList *work;
  GtkWidget *item;
  GtkAdjustment *adj;
  gint new_value;

  g_return_if_fail (list != 0);
  g_return_if_fail (GTK_IS_HLIST (list));

  container = GTK_CONTAINER (list);

  if (container->focus_child)
    work = g_list_find (list->children, container->focus_child);
  else
    work = list->children;

  if (!work)
    return;

  switch (scroll_type)
    {
    case GTK_SCROLL_STEP_BACKWARD:
      work = work->prev;
      if (work)
	gtk_widget_grab_focus (GTK_WIDGET (work->data));
      break;
    case GTK_SCROLL_STEP_FORWARD:
      work = work->next;
      if (work)
	gtk_widget_grab_focus (GTK_WIDGET (work->data));
      break;
    case GTK_SCROLL_PAGE_BACKWARD:
      if (!work->prev)
	return;

      item = work->data;
      adj = gtk_object_get_data_by_id (GTK_OBJECT (list), vadjustment_key_id);

      if (adj)
	{
	  gboolean correct = FALSE;

	  new_value = adj->value;

	  if (item->allocation.y <= adj->value)
	    {
	      new_value = MAX (item->allocation.y + item->allocation.height
			       - adj->page_size, adj->lower);
	      correct = TRUE;
	    }

	  if (item->allocation.y > new_value)
	    for (; work; work = work->prev)
	      {
		item = GTK_WIDGET (work->data);
		if (item->allocation.y <= new_value &&
		    item->allocation.y + item->allocation.height > new_value)
		  break;
	      }
	  else
	    for (; work; work = work->next)
	      {
		item = GTK_WIDGET (work->data);
		if (item->allocation.y <= new_value &&
		    item->allocation.y + item->allocation.height > new_value)
		  break;
	      }

	  if (correct && work && work->next && item->allocation.y < new_value)
	    item = work->next->data;
	}
      else
	item = list->children->data;
	  
      gtk_widget_grab_focus (item);
      break;
    case GTK_SCROLL_PAGE_FORWARD:
      if (!work->next)
	return;

      item = work->data;
      adj = gtk_object_get_data_by_id (GTK_OBJECT (list), vadjustment_key_id);

      if (adj)
	{
	  gboolean correct = FALSE;

	  new_value = adj->value;

	  if (item->allocation.y + item->allocation.height >=
	      adj->value + adj->page_size)
	    {
	      new_value = item->allocation.y;
	      correct = TRUE;
	    }

	  new_value = MIN (new_value + adj->page_size, adj->upper);

	  if (item->allocation.y > new_value)
	    for (; work; work = work->prev)
	      {
		item = GTK_WIDGET (work->data);
		if (item->allocation.y <= new_value &&
		    item->allocation.y + item->allocation.height > new_value)
		  break;
	      }
	  else
	    for (; work; work = work->next)
	      {
		item = GTK_WIDGET (work->data);
		if (item->allocation.y <= new_value &&
		    item->allocation.y + item->allocation.height > new_value)
		  break;
	      }

	  if (correct && work && work->prev &&
	      item->allocation.y + item->allocation.height - 1 > new_value)
	    item = work->prev->data;
	}
      else
	item = g_list_last (work)->data;
	  
      gtk_widget_grab_focus (item);
      break;
    case GTK_SCROLL_JUMP:
      new_value = GTK_WIDGET(list)->allocation.height * CLAMP (position, 0, 1);

      for (item = NULL, work = list->children; work; work =work->next)
	{
	  item = GTK_WIDGET (work->data);
	  if (item->allocation.y <= new_value &&
	      item->allocation.y + item->allocation.height > new_value)
	    break;
	}

      gtk_widget_grab_focus (item);
      break;
    default:
      break;
    }
}

static gint
gtk_hlist_horizontal_timeout (GtkWidget *list)
{
  GdkEventMotion event = { 0 };

  GDK_THREADS_ENTER ();

  GTK_HLIST (list)->htimer = 0;

  event.type = GDK_MOTION_NOTIFY;
  event.send_event = TRUE;

  gtk_hlist_motion_notify (list, &event);

  GDK_THREADS_LEAVE ();

  return FALSE;
}

static gint
gtk_hlist_vertical_timeout (GtkWidget *list)
{
  GdkEventMotion event = { 0 };

  GDK_THREADS_ENTER ();

  GTK_HLIST (list)->vtimer = 0;

  event.type = GDK_MOTION_NOTIFY;
  event.send_event = TRUE;

  gtk_hlist_motion_notify (list, &event);

  GDK_THREADS_LEAVE ();

  return FALSE;
}


/* Private GtkListItem Signal Functions :
 *
 * gtk_hlist_signal_toggle_focus_row
 * gtk_hlist_signal_select_all
 * gtk_hlist_signal_unselect_all
 * gtk_hlist_signal_undo_selection
 * gtk_hlist_signal_start_selection
 * gtk_hlist_signal_end_selection
 * gtk_hlist_signal_extend_selection
 * gtk_hlist_signal_scroll_horizontal
 * gtk_hlist_signal_scroll_vertical
 * gtk_hlist_signal_toggle_add_mode
 * gtk_hlist_signal_item_select
 * gtk_hlist_signal_item_deselect
 * gtk_hlist_signal_item_toggle
 */
static void
gtk_hlist_signal_toggle_focus_row (GtkListItem *list_item,
				  GtkHList     *list)
{
  g_return_if_fail (list_item != 0);
  g_return_if_fail (GTK_IS_LIST_ITEM (list_item));
  g_return_if_fail (list != NULL);
  g_return_if_fail (GTK_IS_HLIST (list));

  gtk_hlist_toggle_focus_row (list);
}

static void
gtk_hlist_signal_select_all (GtkListItem *list_item,
			    GtkHList     *list)
{
  g_return_if_fail (list_item != 0);
  g_return_if_fail (GTK_IS_LIST_ITEM (list_item));
  g_return_if_fail (list != NULL);
  g_return_if_fail (GTK_IS_HLIST (list));

  gtk_hlist_select_all (list);
}

static void
gtk_hlist_signal_unselect_all (GtkListItem *list_item,
			      GtkHList     *list)
{
  g_return_if_fail (list_item != 0);
  g_return_if_fail (GTK_IS_LIST_ITEM (list_item));
  g_return_if_fail (list != NULL);
  g_return_if_fail (GTK_IS_HLIST (list));

  gtk_hlist_unselect_all (list);
}

static void
gtk_hlist_signal_undo_selection (GtkListItem *list_item,
				GtkHList     *list)
{
  g_return_if_fail (list_item != 0);
  g_return_if_fail (GTK_IS_LIST_ITEM (list_item));
  g_return_if_fail (list != NULL);
  g_return_if_fail (GTK_IS_HLIST (list));

  gtk_hlist_undo_selection (list);
}

static void
gtk_hlist_signal_start_selection (GtkListItem *list_item,
				 GtkHList     *list)
{
  g_return_if_fail (list_item != 0);
  g_return_if_fail (GTK_IS_LIST_ITEM (list_item));
  g_return_if_fail (list != NULL);
  g_return_if_fail (GTK_IS_HLIST (list));

  gtk_hlist_start_selection (list);
}

static void
gtk_hlist_signal_end_selection (GtkListItem *list_item,
			       GtkHList     *list)
{
  g_return_if_fail (list_item != 0);
  g_return_if_fail (GTK_IS_LIST_ITEM (list_item));
  g_return_if_fail (list != NULL);
  g_return_if_fail (GTK_IS_HLIST (list));

  gtk_hlist_end_selection (list);
}

static void
gtk_hlist_signal_extend_selection (GtkListItem   *list_item,
				  GtkScrollType  scroll_type,
				  gfloat         position,
				  gboolean       auto_start_selection,
				  GtkHList       *list)
{
  g_return_if_fail (list_item != 0);
  g_return_if_fail (GTK_IS_LIST_ITEM (list_item));
  g_return_if_fail (list != NULL);
  g_return_if_fail (GTK_IS_HLIST (list));

  gtk_hlist_extend_selection (list, scroll_type, position,
			     auto_start_selection);
}

static void
gtk_hlist_signal_scroll_horizontal (GtkListItem   *list_item,
				   GtkScrollType  scroll_type,
				   gfloat         position,
				   GtkHList       *list)
{
  g_return_if_fail (list_item != 0);
  g_return_if_fail (GTK_IS_LIST_ITEM (list_item));
  g_return_if_fail (list != NULL);
  g_return_if_fail (GTK_IS_HLIST (list));

  gtk_hlist_scroll_horizontal (list, scroll_type, position);
}

static void
gtk_hlist_signal_scroll_vertical (GtkListItem   *list_item,
				 GtkScrollType  scroll_type,
				 gfloat         position,
				 GtkHList       *list)
{
  g_return_if_fail (list_item != 0);
  g_return_if_fail (GTK_IS_LIST_ITEM (list_item));
  g_return_if_fail (list != NULL);
  g_return_if_fail (GTK_IS_HLIST (list));

  gtk_hlist_scroll_vertical (list, scroll_type, position);
}

static void
gtk_hlist_signal_toggle_add_mode (GtkListItem *list_item,
				 GtkHList     *list)
{
  g_return_if_fail (list_item != 0);
  g_return_if_fail (GTK_IS_LIST_ITEM (list_item));
  g_return_if_fail (list != NULL);
  g_return_if_fail (GTK_IS_HLIST (list));

  gtk_hlist_toggle_add_mode (list);
}

static void
gtk_hlist_signal_item_select (GtkListItem *list_item,
			     GtkHList     *list)
{
  GList *selection;
  GList *tmp_list;
  GList *sel_list;

  g_return_if_fail (list_item != 0);
  g_return_if_fail (GTK_IS_LIST_ITEM (list_item));
  g_return_if_fail (list != NULL);
  g_return_if_fail (GTK_IS_HLIST (list));

  if (GTK_WIDGET (list_item)->state != GTK_STATE_SELECTED)
    return;

  switch (list->selection_mode)
    {
    case GTK_SELECTION_SINGLE:
    case GTK_SELECTION_BROWSE:
      sel_list = NULL;
      selection = list->selection;

      while (selection)
	{
	  tmp_list = selection;
	  selection = selection->next;

	  if (tmp_list->data == list_item)
	    sel_list = tmp_list;
	  else
	    gtk_list_item_deselect (GTK_LIST_ITEM (tmp_list->data));
	}

      if (!sel_list)
	{
	  list->selection = g_list_prepend (list->selection, list_item);
	  gtk_widget_ref (GTK_WIDGET (list_item));
	}
      gtk_signal_emit (GTK_OBJECT (list), list_signals[SELECTION_CHANGED]);
      break;
    case GTK_SELECTION_EXTENDED:
      if (list->anchor >= 0)
	return;
    case GTK_SELECTION_MULTIPLE:
      if (!g_list_find (list->selection, list_item))
	{
	  list->selection = g_list_prepend (list->selection, list_item);
	  gtk_widget_ref (GTK_WIDGET (list_item));
	  gtk_signal_emit (GTK_OBJECT (list), list_signals[SELECTION_CHANGED]);
	}
      break;
    }
}

static void
gtk_hlist_signal_item_deselect (GtkListItem *list_item,
			       GtkHList     *list)
{
  GList *node;

  g_return_if_fail (list_item != 0);
  g_return_if_fail (GTK_IS_LIST_ITEM (list_item));
  g_return_if_fail (list != NULL);
  g_return_if_fail (GTK_IS_HLIST (list));

  if (GTK_WIDGET (list_item)->state != GTK_STATE_NORMAL)
    return;

  node = g_list_find (list->selection, list_item);

  if (node)
    {
      list->selection = g_list_remove_link (list->selection, node);
      g_list_free_1 (node);
      gtk_widget_unref (GTK_WIDGET (list_item));
      gtk_signal_emit (GTK_OBJECT (list), list_signals[SELECTION_CHANGED]);
    }
}

static void
gtk_hlist_signal_item_toggle (GtkListItem *list_item,
			     GtkHList     *list)
{
  g_return_if_fail (list_item != 0);
  g_return_if_fail (GTK_IS_LIST_ITEM (list_item));
  g_return_if_fail (list != NULL);
  g_return_if_fail (GTK_IS_HLIST (list));

  if ((list->selection_mode == GTK_SELECTION_BROWSE ||
       list->selection_mode == GTK_SELECTION_EXTENDED) &&
      GTK_WIDGET (list_item)->state == GTK_STATE_NORMAL)
    {
      gtk_widget_set_state (GTK_WIDGET (list_item), GTK_STATE_SELECTED);
      return;
    }
  
  switch (GTK_WIDGET (list_item)->state)
    {
    case GTK_STATE_SELECTED:
      gtk_hlist_signal_item_select (list_item, list);
      break;
    case GTK_STATE_NORMAL:
      gtk_hlist_signal_item_deselect (list_item, list);
      break;
    default:
      break;
    }
}

static void
gtk_hlist_signal_drag_begin (GtkWidget      *widget,
			    GdkDragContext *context,
			    GtkHList	    *list)
{
  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_LIST_ITEM (widget));
  g_return_if_fail (list != NULL);
  g_return_if_fail (GTK_IS_HLIST (list));

  gtk_hlist_drag_begin (GTK_WIDGET (list), context);
}

static void
gtk_hlist_drag_begin (GtkWidget      *widget,
		     GdkDragContext *context)
{
  GtkHList *list;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_HLIST (widget));
  g_return_if_fail (context != NULL);

  list = GTK_HLIST (widget);

  if (list->drag_selection)
    {
      gtk_hlist_end_drag_selection (list);

      switch (list->selection_mode)
	{
	case GTK_SELECTION_EXTENDED:
	  gtk_hlist_end_selection (list);
	  break;
	case GTK_SELECTION_SINGLE:
	case GTK_SELECTION_MULTIPLE:
	  list->undo_focus_child = NULL;
	  break;
	default:
	  break;
	}
    }
}
