import type { CSSProperties } from 'react'

// Shared style tokens — ported from the handoff's renderVals() (OP4 App.dc.html).
// Every color references a CSS custom property, so these restyle with the theme.

export const dispChip: CSSProperties = { font: "500 13px 'IBM Plex Mono'", color: 'var(--disp-ink)', background: 'var(--disp)', border: '1px solid var(--disp-b)', borderRadius: '3px', padding: '2px 7px', boxShadow: 'var(--disp-inset)' }
export const dispChipSm: CSSProperties = { font: "500 11px 'IBM Plex Mono'", color: 'var(--disp-ink)', background: 'var(--disp)', border: '1px solid var(--disp-b)', borderRadius: '3px', padding: '2px 10px', boxShadow: 'var(--disp-inset)', textAlign: 'center' }
export const dispChipName: CSSProperties = { font: "500 11px 'IBM Plex Mono'", color: 'var(--disp-ink)', background: 'var(--disp)', border: '1px solid var(--disp-b)', borderRadius: '3px', padding: '3px 8px', boxShadow: 'var(--disp-inset)', whiteSpace: 'nowrap', overflow: 'hidden' }
export const keyBtn: CSSProperties = { padding: '4px 11px', borderRadius: '3px', font: "600 10px 'Barlow Condensed'", letterSpacing: '.12em', color: 'var(--key-ink)', background: 'var(--key)', border: '1px solid var(--key-b)', boxShadow: '0 2px 3px rgba(0,0,0,.3)' }
export const keyABtn: CSSProperties = { padding: '4px 11px', borderRadius: '3px', font: "600 10px 'Barlow Condensed'", letterSpacing: '.12em', color: 'var(--keyA-ink)', background: 'var(--keyA)', border: '1px solid var(--keyA-b)', boxShadow: '0 2px 3px rgba(0,0,0,.3)' }
export const spin: CSSProperties = { width: '16px', height: '11px', display: 'flex', alignItems: 'center', justifyContent: 'center', background: 'var(--key)', border: '1px solid var(--key-b)', borderRadius: '2px', font: "600 7px 'IBM Plex Mono'", color: 'var(--key-ink)' }
export const colHd: CSSProperties = { font: "600 8.5px 'Barlow Condensed'", letterSpacing: '.14em', color: 'var(--faint)' }
export const chipKey: CSSProperties = { font: "500 10px 'IBM Plex Mono'", color: 'var(--key-ink)', background: 'var(--key)', border: '1px solid var(--key-b)', borderRadius: '4px', padding: '3px 8px', boxShadow: '0 1px 2px rgba(0,0,0,.3)' }
export const chipKeyA: CSSProperties = { font: "500 9px 'IBM Plex Mono'", color: 'var(--keyA-ink)', background: 'var(--keyA)', border: '1px solid var(--keyA-b)', borderRadius: '4px', padding: '2px 6px', textAlign: 'center' }
export const chipKeySm: CSSProperties = { font: "600 9px 'Barlow Condensed'", letterSpacing: '.1em', color: 'var(--key-ink)', background: 'var(--key)', border: '1px solid var(--key-b)', borderRadius: '3px', padding: '2px 8px', opacity: 0.75 }
export const chipKeyASm: CSSProperties = { font: "600 9px 'Barlow Condensed'", letterSpacing: '.1em', color: 'var(--keyA-ink)', background: 'var(--keyA)', border: '1px solid var(--keyA-b)', borderRadius: '3px', padding: '2px 8px' }
export const segOn: CSSProperties = { padding: '4px 10px', borderRadius: '3px', font: "600 10px 'Barlow Condensed'", letterSpacing: '.1em', color: 'var(--keyA-ink)', background: 'var(--keyA)', border: '1px solid var(--keyA-b)' }
export const segOff: CSSProperties = { padding: '4px 10px', borderRadius: '3px', font: "600 10px 'Barlow Condensed'", letterSpacing: '.1em', color: 'var(--key-ink)', background: 'var(--key)', border: '1px solid var(--key-b)', opacity: 0.75 }
export const segOnF: CSSProperties = { ...segOn, flex: '1', textAlign: 'center' }
export const segOffF: CSSProperties = { ...segOff, flex: '1', textAlign: 'center' }
export const segOnSm: CSSProperties = { padding: '3px 9px', borderRadius: '3px', font: "600 9.5px 'Barlow Condensed'", letterSpacing: '.1em', color: 'var(--keyA-ink)', background: 'var(--keyA)', border: '1px solid var(--keyA-b)' }
export const segOffSm: CSSProperties = { padding: '3px 9px', borderRadius: '3px', font: "600 9.5px 'Barlow Condensed'", letterSpacing: '.1em', color: 'var(--key-ink)', background: 'var(--key)', border: '1px solid var(--key-b)', opacity: 0.75 }
export const toggleOn: CSSProperties = { width: '30px', height: '15px', borderRadius: '8px', background: 'var(--keyA)', position: 'relative', boxShadow: 'inset 0 1px 2px rgba(0,0,0,.3)', flex: 'none' }
export const toggleKnob: CSSProperties = { position: 'absolute', right: '1px', top: '1px', width: '13px', height: '13px', borderRadius: '50%', background: '#f2ecd9', boxShadow: '0 1px 2px rgba(0,0,0,.4)' }

// Section card shell used across every view.
export const panelCard: CSSProperties = { background: 'var(--panel)', border: '1px solid var(--panel-b)', borderRadius: '6px' }

// Small shared text roles.
export const sectionLabel: CSSProperties = { font: "600 10px 'Barlow Condensed'", letterSpacing: '.16em', color: 'var(--silk)' }
export const paramLabel: CSSProperties = { font: "600 9px 'Barlow Condensed'", letterSpacing: '.14em', color: 'var(--silk)' }

/** Knob pointer rotation: value 0..99 → degrees, per the handoff (`v/99*270 - 135`). */
export const deg = (v: number): number => Math.round((v / 99) * 270 - 135)
