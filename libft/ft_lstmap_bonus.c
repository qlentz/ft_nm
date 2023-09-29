/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_lstmap.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qlentz <marvin@42lausanne.ch>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/10/10 14:00:09 by qlentz            #+#    #+#             */
/*   Updated: 2022/10/11 13:40:29 by qlentz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

t_list	*ft_lstmap(t_list *lst, void *(*f)(void *), void (*del)(void *))
{
	t_list	*newlst;
	t_list	*newelem;
	t_list	*temp;

	temp = lst;
	if (!lst || !(*f))
		return (NULL);
	newlst = NULL;
	while (lst)
	{
		newelem = ft_lstnew((*f)(lst->content));
		if (!newelem)
		{
			ft_lstclear(&newlst, (*del));
			break ;
		}
		ft_lstadd_back(&newlst, newelem);
		lst = lst->next;
	}
	lst = temp;
	return (newlst);
}
