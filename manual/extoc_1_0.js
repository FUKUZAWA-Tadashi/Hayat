/*
 * 	exTOC 1.0 - jQuery plugin
 *	written by Cyokodog	
 *
 *	Copyright (c) 2009 Cyokodog (http://d.hatena.ne.jp/cyokodog/)
 *	Dual licensed under the MIT (MIT-LICENSE.txt)
 *	and GPL (GPL-LICENSE.txt) licenses.
 *
 *	Built for jQuery library
 *	http://jquery.com
 *
 */
(function($j){
	$j.ex=$j.ex||{};

	/*
	 * 	ex.defineExPlugin 1.0
	 */
	$j.ex.defineExPlugin = function( pluginName , constr ){
		$j.fn[ pluginName ]=
			function( cfg ){
				var o = this,arr = [];
				o.each(function( idx ){
					arr.push(new constr( o.eq(idx) , cfg ));
				});
				var obj = $j(arr);
				for(var i in constr.prototype)( function(i){
					if(i.slice(0,1)!= '_'){
						obj[i] = function(){
							return obj[0][i].apply( obj[0] , arguments );
						}
					}
				})(i);
				obj.target = function(){ return o }
				return obj;
			};
	}

	/*
	 * 	ex.margeArray 1.0
	 */
	$j.ex.margeArray = function( to , from ){
		var l = to.length;
		for(var i = 0;i < from.length; i++)to[l+i] = from[i]	
		return to;
	}

	/*
	 * 	ex.filterAndFind 1.0
	 */
	$j.ex.filterAndFind = function(expr,nodes){
		var arr1 = $j.makeArray( nodes.filter(expr) );
		var arr2 = $j.makeArray( nodes.find(expr) );
		return $j($j.ex.margeArray( arr1 , arr2 ));
	}

	/*
	 * 	ex.headEach 1.0
	 */
	$j.ex.headEach = function(nodes,lv,nums,data,c){
		var heads;
		while(
			(heads = $j.ex.filterAndFind('h'+lv,nodes)).size() == 0 && lv < c.headTo
		){lv++;};
	
		var nums = $j.ex.margeArray([],nums);
		nums.push(0);
	
		var data = $j.extend({},data);
		if(c.callback({
				headNodes : heads,
				level : nums
			},data,c
		) == false) return;
	
		heads.each(function(idx){
	
			var head = heads.eq(idx);
			nums[nums.length-1]++;
	
			if(c.each({
					index : idx,
					headNode : head,
					level : nums
				},data,c
			) == false || lv >= c.headTo)return;
	
			var nAll = head.nextAll(),rEnd = false,rNode = [];
			nAll.each(function(idx){
				var node = nAll.eq(idx);
				var tName = node[0].tagName;
				if(/H1|H2|H3|H4|H5|H6/i.test(tName) && tName.slice(1,2)*1<=lv){
					rEnd = true;
				}
				if(!rEnd){
					rNode.push(node[0]);
				}
			})
			if(rNode.length>0){
				$j.ex.headEach($j(rNode),lv+1,nums,data,c);
			}
		})
	}
	$j.fn.exHeadEach = function(cfg){
		var c = $j.extend({
			headFrom : 1,
			headTo : 6,
			callback : function(){},
			each : function(){}
		},cfg||{});
		$j.ex.headEach(this,c.headFrom,[],{},c);
		return this;
	}

	/*
	 * 	ex.TOC 1.0
	 */
	$j.ex.TOC = function(target,cfg){
		var o = this,c = o.cfg = $j.extend({
			container : target,
			contents : 'body',
			headFrom : 3,
			headTo : 5,
			index : true,
			numbering : true,
			numberingHead : false,
			numberingFrom : 1,
			numberingTo : 6,
			insertMethod : 'prepend',
			link : true,
			smooth : true,
			easing : 'swing',
			speed : 'slow',
			contentsScroller : $j.boxModel && !$.browser.safari ? $j('html') : $j('body')
		},cfg);

		c.contents = $j(c.contents);

		c.contents.exHeadEach({
			headFrom : c.headFrom,
			headTo : c.headTo,
			callback : function( prm , data , cfg ){
				if( !data.ctnr )data.ctnr = c.container;
				if(prm.headNodes.size()==0)return;
				data.grp = o._insGroup( data.ctnr , prm.level );
				if(prm.level.length==1)c.toc = data.grp;
			},
			each : function( prm , data ,c ){
				data.ctnr = o._insContainer( data.grp , prm.headNode , prm.level );
			}
		})
	}
	$j.extend($j.ex.TOC.prototype,{
		getTOC : function(){
			return this.cfg.toc;			
		},
		_getNText : function(tag,text,cls,prp){
			return '<' + tag + ' class="' + cls + '" ' + (prp?prp:'') + '>' + text + '</' + tag + '>';
		},
		_getTOCNtext : function(tag , text , fix , lv , prp ){
			return this._getNText(tag,text,'ex-toc' + fix + ' ex-toc' + fix + '-' + lv, prp);
		},
		_getUID : function(){
			return 'ex-toc-uid'+($j.ex.TOC.cfg.uID++);
		},
		_getLabel : function(level,forHead){
			var o = this,c = o.cfg;
			if(!c.numbering || (forHead && !c.numberingHead) ||
				(level.length > c.numberingTo))return '';
	
			var label = '';
			for(var i = c.numberingFrom-1;i < Math.min(c.numberingTo,level.length);i++){
				label += level[i] + '.';
			}
			return label;
		},
		_insGroup : function(cntr,nums){
			var o = this,c = o.cfg;
			var nlen = nums.length;
			var cls = nlen == 1?'ex-toc ':'';
			cls += 'ex-toc-' + nlen;
		
			var grp = $j(o._getNText('ul','',cls));
	
			if(c.index)cntr[nlen-1 ? 'append' : c.insertMethod](grp);
			return grp;
		},
		_insContainer : function(grp,headNode,level){
			var o = this,c = o.cfg;
			var nlen = level.length;
			var baseCls = 'ex-toc';
			var cls = baseCls + '-' + nlen;
		
			var id = headNode[0].id;
			if(!id)headNode.attr('id',id = o._getUID());
	
			var label = o._getTOCNtext('span',o._getLabel(level) , '-label' , nlen)
			var text = o._getTOCNtext('span',headNode.text() , '-text' , nlen)

			var link = 
				!c.link ? $j(text) : 
				$j(o._getTOCNtext('a',label + text , '' , nlen ,c.link ? 'href="#' + id + '"' : ''));

			var cntr = $j(o._getTOCNtext('li','' , '' , nlen));

			link.appendTo(cntr)
	
			o._bindSmoothLink(link,headNode);
	
			if(c.index)grp.append(cntr);
		
			var label = o._getNText('span',o._getLabel(level,true),cls);
			$j(label).prependTo(headNode);

			return cntr;
		},
		_bindSmoothLink : function(link,head){
			var o = this,c = o.cfg;
			if( !c.link || !c.smooth )return;
			link.click( function(){
				c.contentsScroller.animate({
					'scrollTop' : head.offset().top
				}, c.speed , c.easing )
				return false;
			})
		}
	});
	
	$j.ex.TOC.cfg = {
		uID:1000	
	}
	
	$j.ex.defineExPlugin('exTOC',$j.ex.TOC);

})(jQuery);

